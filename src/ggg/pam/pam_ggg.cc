#include <stdexcept>

#include <security/pam_ext.h>
#include <security/pam_modules.h>
#include <unistd.h>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/ctl/password.hh>
#include <ggg/pam/pam_handle.hh>
#include <ggg/sec/argon2.hh>
#include <ggg/sec/secure_string.hh>

using ggg::throw_pam_error;
using ggg::pam_errc;
using ggg::pam::call;
using ggg::pam_category;

namespace {

	typedef std::chrono::duration<long,std::ratio<60*60*24,1>> days;

	ggg::account::clock_type::rep
	num_days_since_expired(
		const ggg::account* acc,
		ggg::account::time_point now
	) {
		using std::chrono::duration_cast;
		return duration_cast<days>(now - acc->expire()).count();
	}

	ggg::account::clock_type::rep
	num_days_since_password_expired(
		const ggg::account* acc,
		ggg::account::time_point now
	) {
		using std::chrono::duration_cast;
		const auto tmp = now - acc->last_change() - acc->max_change();
		return duration_cast<days>(tmp).count();
	}

	ggg::account
	find_account(ggg::Database& db, const char* user) {
		auto rstr = db.find_account(user);
		ggg::account_iterator first(rstr), last;
		if (first == last) {
			throw_pam_error(ggg::pam_errc::unknown_user);
		}
		return *first;
	}

	void
	check_password(const std::string& hashed_password, const char* password) {
        if (!ggg::verify_password(hashed_password, password)) {
            throw_pam_error(pam_errc::permission_denied);
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

}

int pam_sm_authenticate(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
    using namespace ggg;
    using ggg::pam_handle;
	pam_errc ret = pam_errc::ignore;
	pam_handle pamh(orig, argc, argv);
	try {
		const char* user = pamh.get_user();
		pamh.debug("authenticating user \"%s\"", user);
		const char* password = pamh.get_password(pam_errc::authentication_error);
        auto& db = pamh.get_database();
        Transaction tr(db);
		account acc = find_account(db, user);
		check_password(acc.password(), password);
        const auto& msg = pamh.get_message();
        std::string argon2_prefix = "$argon2id$";
        if (acc.password().compare(0, argon2_prefix.size(), argon2_prefix) != 0) {
            pamh.debug("migrating to argon2 for user \"%s\"", user);
            migrate_to_argon2(db, acc, password);
            pamh.debug("successfully migrated to argon2 for user \"%s\"", user);
            db.message(user, msg.timestamp, msg.hostname, "migrated to argon2");
        }
		pamh.set_account(acc);
		pamh.debug("successfully authenticated user \"%s\"", user);
        db.message(user, msg.timestamp, msg.hostname, "authenticated");
        tr.commit();
		ret = pam_errc::success;
	} catch (const std::system_error& e) {
		ret = pamh.handle_error(e, pam_errc::authentication_error);
	} catch (const std::bad_alloc& e) {
		ret = pamh.handle_error(e);
	} catch (const std::exception& e) {
		ret = pamh.handle_error(e);
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
	pam_errc ret = pam_errc::ignore;
	pam_handle pamh(orig, argc, argv);
	account other;
	try {
		const char* user = pamh.get_user();
		pamh.debug("checking account \"%s\"", user);
		const account* acc = nullptr;
		try {
			acc = pamh.get_account();
		} catch (const std::system_error& err) {
            auto& db = pamh.get_database();
			other = find_account(db, user);
			acc = &other;
		}
		if (acc->has_been_suspended()) {
			pamh.debug("account \"%s\" has been suspended", user);
			throw_pam_error(pam_errc::permission_denied);
		}
		const account::time_point now = account::clock_type::now();
		if (acc->has_expired(now)) {
			pamh.debug(
				"account \"%s\" has expired %ld day(s) ago",
				user, num_days_since_expired(acc, now)
			);
			throw_pam_error(pam_errc::account_expired);
		}
		if (acc->password_has_expired(now)) {
			pamh.info("Password has expired.");
			pamh.debug(
				"password of account \"%s\" has expired %ld day(s) ago",
				user, num_days_since_password_expired(acc, now)
			);
			throw_pam_error(pam_errc::new_password_required);
		}
		pamh.debug("account \"%s\" is valid", user);
		ret = pam_errc::success;
	} catch (const std::system_error& e) {
		ret = pamh.handle_error(e, pam_errc::authtok_error);
	} catch (const std::bad_alloc& e) {
		ret = pamh.handle_error(e);
	} catch (const std::exception& e) {
		ret = pamh.handle_error(e);
	}
	return std::make_error_condition(ret).value();
}

int pam_sm_chauthtok(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
	pam_errc ret = pam_errc::ignore;
	ggg::pam_handle pamh(orig, argc, argv);
	if (flags & PAM_PRELIM_CHECK) {
		ret = pam_errc::success;
	} else if (flags & PAM_UPDATE_AUTHTOK) {
        using namespace ggg;
		try {
			pamh.set_password_type("GGG");
			const char* user = pamh.get_user();
			pamh.debug("changing password for user \"%s\"", user);
            auto& db = pamh.get_database();
			account acc = find_account(db, user);
			const account::time_point now = account::clock_type::now();
			if (acc.has_expired(now)) {
				pamh.debug(
					"account \"%s\" has expired %ld day(s) ago",
					user, num_days_since_expired(&acc, now)
				);
				throw_pam_error(pam_errc::account_expired);
			}
			if ((flags & PAM_CHANGE_EXPIRED_AUTHTOK) &&
				!acc.password_has_expired(now))
			{
				pamh.debug(
					"trying to change expired password for user \"%s\","
					" but the password has not expired yet!", user
				);
				throw_pam_error(pam_errc::authtok_error);
			}
            auto uid = ::getuid();
			if (uid != 0) {
				const char* old = pamh.get_old_password();
				check_password(acc.password(), old);
			}
			const char* new_password = pamh.get_password(pam_errc::authtok_error);
			if (uid != 0) {
			    try {
			    	validate_password(new_password, pamh.min_entropy());
			    } catch (const std::exception& err) {
			    	pamh.error("%s", err.what());
			    	throw_pam_error(pam_errc::authtok_error);
			    }
            }
            init_sodium();
            argon2_password_hash hash;
			acc.set_password(hash(new_password));
			db.set_password(acc);
			pamh.debug("successfully changed password for user \"%s\"", user);
            const auto& msg = pamh.get_message();
            db.message(user, msg.timestamp, msg.hostname, "changed password");
			ret = pam_errc::success;
		} catch (const std::system_error& e) {
			ret = pamh.handle_error(e, pam_errc::authtok_error);
		} catch (const std::bad_alloc& e) {
			ret = pamh.handle_error(e);
		} catch (const std::exception& e) {
			ret = pamh.handle_error(e);
		}
	}
	return std::make_error_condition(ret).value();
}

// unused {{{
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
	pam_errc ret = pam_errc::ignore;
	ggg::pam_handle pamh(orig, argc, argv);
    try {
		const char* user = pamh.get_user();
        const auto& msg = pamh.get_message();
        auto& db = pamh.get_database();
        db.message(user, msg.timestamp, msg.hostname, "session opened");
        ret = pam_errc::success;
    } catch (const std::exception& e) {
        ret = pamh.handle_error(e);
    }
	return std::make_error_condition(ret).value();
}

int pam_sm_close_session(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
	pam_errc ret = pam_errc::ignore;
	ggg::pam_handle pamh(orig, argc, argv);
    try {
		const char* user = pamh.get_user();
        const auto& msg = pamh.get_message();
        auto& db = pamh.get_database();
        db.message(user, msg.timestamp, msg.hostname, "session closed");
        ret = pam_errc::success;
    } catch (const std::exception& e) {
        ret = pamh.handle_error(e);
    }
	return std::make_error_condition(ret).value();
}
// }}}
