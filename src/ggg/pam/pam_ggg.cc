#include <stdexcept>

#include <security/pam_ext.h>
#include <security/pam_modules.h>
#include <unistd.h>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/pam/pam_handle.hh>
#include <ggg/sec/argon2.hh>
#include <ggg/sec/password.hh>
#include <ggg/sec/secure_string.hh>

using pam::throw_pam_error;
using pam::errc;
using pam::call;
using pam::pam_category;

namespace {

	ggg::account
	find_account(ggg::Database& db, const char* user) {
		auto rstr = db.find_account(user);
		ggg::account_iterator first(rstr), last;
		if (first == last) { throw_pam_error(errc::unknown_user); }
		return *first;
	}

	void
	check_password(const std::string& hashed_password, const char* password) {
        if (!ggg::verify_password(hashed_password, password)) {
            throw_pam_error(errc::permission_denied);
        }
	}

    void
    migrate_to_argon2(ggg::Database& db, ggg::account& acc, const char* password) {
        using namespace ggg;
        init_sodium();
        argon2_password_hash hash;
        acc.set_password(hash(password));
        db.set_password(acc);
    }

    std::string collect_data(pam::handle pamh) {
        std::string data;
        data.reserve(1024);
        const char* value = nullptr;
        if (PAM_SUCCESS == pamh.get_item(PAM_RUSER, &value) && value) {
            data += " data ";
            data += value;
            if (PAM_SUCCESS == pamh.get_item(PAM_RHOST, &value) && value) {
                data += '@';
                data += value;
            }
        }
        if (PAM_SUCCESS == pamh.get_item(PAM_SERVICE, &value) && value) {
            data += " service ";
            data += value;
        }
        if (PAM_SUCCESS == pamh.get_item(PAM_TTY, &value) && value) {
            data += " tty ";
            data += value;
            if (PAM_SUCCESS == pamh.get_item(PAM_XDISPLAY, &value) && value) {
                data += " xdisplay ";
                data += value;
            }
        }
        return data;
    }

}

int pam_sm_authenticate(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
    using namespace ggg;
    using ggg::pam_handle;
	errc ret = errc::ignore;
	pam_handle pamh(orig, argc, argv);
	try {
		const char* user = pamh.user();
		pamh.debug("authenticating user \"%s\"", user);
		const char* password = pamh.password(errc::authentication_error);
        auto& db = *pamh.get_database();
        Transaction tr(db);
		account acc = find_account(db, user);
		check_password(acc.password(), password);
        std::string argon2_prefix = "$argon2id$";
        if (acc.password().compare(0, argon2_prefix.size(), argon2_prefix) != 0) {
            pamh.debug("migrating to argon2 for user \"%s\"", user);
            migrate_to_argon2(db, acc, password);
            pamh.debug("successfully migrated to argon2 for user \"%s\"", user);
            db.message(user, "migrated to argon2");
        }
		pamh.set_account(acc);
		pamh.debug("successfully authenticated user \"%s\"", user);
        db.message(user, "authenticated");
        tr.commit();
		ret = errc::success;
	} catch (const std::system_error& e) {
		ret = pamh.error(e, errc::authentication_error);
	} catch (const std::bad_alloc& e) {
		ret = pamh.error(e);
	} catch (const std::exception& e) {
		ret = pamh.error(e);
	}
	return std::make_error_condition(ret).value();
}

int pam_sm_acct_mgmt(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
    using namespace ggg;
    using ggg::pam_handle;
	errc ret = errc::ignore;
	pam_handle pamh(orig, argc, argv);
	account other;
	try {
		const char* user = pamh.user();
		pamh.debug("checking account \"%s\"", user);
		account* acc = nullptr;
        Database* db = nullptr;
		try {
			acc = pamh.get_account();
		} catch (const std::system_error& err) {
            db = pamh.get_database();
			other = find_account(*db, user);
			acc = &other;
		}
		if (acc->has_been_suspended()) {
			pamh.debug("account \"%s\" has been suspended", user);
			throw_pam_error(errc::permission_denied);
		}
		const account::time_point now = account::clock_type::now();
		if (acc->has_expired(now)) {
			pamh.debug("account \"%s\" has expired", user);
			throw_pam_error(errc::account_expired);
		}
		if (acc->is_inactive(now)) {
			pamh.debug("account \"%s\" has been inactive for too long", user);
			throw_pam_error(errc::account_expired);
		}
		if (acc->password_has_expired(now)) {
			pamh.info("Password has expired.");
			pamh.debug("password of account \"%s\" has expired", user);
			throw_pam_error(errc::new_password_required);
		}
		pamh.debug("account \"%s\" is valid", user);
        if (!db) { db = pamh.get_database(); }
        acc->last_active(now);
        db->set_last_active(*acc, now);
		ret = errc::success;
	} catch (const std::system_error& e) {
		ret = pamh.error(e, errc::authtok_error);
	} catch (const std::bad_alloc& e) {
		ret = pamh.error(e);
	} catch (const std::exception& e) {
		ret = pamh.error(e);
	}
	return std::make_error_condition(ret).value();
}

int pam_sm_chauthtok(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
	errc ret = errc::ignore;
	ggg::pam_handle pamh(orig, argc, argv);
	if (flags & PAM_PRELIM_CHECK) {
		ret = errc::success;
	} else if (flags & PAM_UPDATE_AUTHTOK) {
        using namespace ggg;
		try {
			pamh.password_type("GGG");
			const char* user = pamh.user();
			pamh.debug("changing password for user \"%s\"", user);
            auto& db = *pamh.get_database();
			account acc = find_account(db, user);
			const account::time_point now = account::clock_type::now();
			if (acc.has_expired(now)) {
				pamh.debug("account \"%s\" has expired", user);
				throw_pam_error(errc::account_expired);
			}
            if (acc.is_inactive(now)) {
                pamh.debug("account \"%s\" has been inactive for too long", user);
                throw_pam_error(errc::account_expired);
            }
			if ((flags & PAM_CHANGE_EXPIRED_AUTHTOK) &&
				!acc.password_has_expired(now))
			{
				pamh.debug(
					"trying to change expired password for user \"%s\","
					" but the password has not expired yet!", user
				);
				throw_pam_error(errc::authtok_error);
			}
            auto uid = ::getuid();
			if (uid != 0) {
				const char* old = pamh.old_password();
				check_password(acc.password(), old);
			}
			const char* new_password = pamh.password(errc::authtok_error);
			if (uid != 0) {
			    try {
			    	validate_password(new_password, pamh.min_entropy());
			    } catch (const std::exception& err) {
			    	pamh.error("%s", err.what());
			    	throw_pam_error(errc::authtok_error);
			    }
            }
            init_sodium();
            argon2_password_hash hash;
			acc.set_password(hash(new_password));
			db.set_password(acc);
			pamh.debug("successfully changed password for user \"%s\"", user);
			ret = errc::success;
		} catch (const std::system_error& e) {
			ret = pamh.error(e, errc::authtok_error);
		} catch (const std::bad_alloc& e) {
			ret = pamh.error(e);
		} catch (const std::exception& e) {
			ret = pamh.error(e);
		}
	}
	return std::make_error_condition(ret).value();
}

int pam_sm_setcred(
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	return PAM_SUCCESS;
}

int pam_sm_open_session(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
	errc ret = errc::ignore;
	ggg::pam_handle pamh(orig, argc, argv);
    try {
		const char* user = pamh.user();
        auto& db = *pamh.get_database();
        db.message(user, "session opened: %s", collect_data(pamh));
        ret = errc::success;
    } catch (const std::exception& e) {
        ret = pamh.error(e);
    }
	return std::make_error_condition(ret).value();
}

int pam_sm_close_session(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
	errc ret = errc::ignore;
	ggg::pam_handle pamh(orig, argc, argv);
    try {
		const char* user = pamh.user();
        auto& db = *pamh.get_database();
        db.message(user, "session closed: %s", collect_data(pamh));
        ret = errc::success;
    } catch (const std::exception& e) {
        ret = pamh.error(e);
    }
	return std::make_error_condition(ret).value();
}
