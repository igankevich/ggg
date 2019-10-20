#include <stdexcept>

#include <security/pam_ext.h>
#include <security/pam_modules.h>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/ctl/password.hh>
#include <ggg/pam/pam_handle.hh>
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
	check_password(const char* password, const ggg::account& acc) {
        ggg::init_sodium();
		if (!ggg::sha512_password_hash::verify(acc.password(), password)) {
			throw_pam_error(pam_errc::permission_denied);
		}
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
		Database db(Database::File::Accounts, Database::Flag::Read_only);
		account acc = find_account(db, user);
		db.close();
		check_password(password, acc);
		pamh.set_account(acc);
		pamh.debug("successfully authenticated user \"%s\"", user);
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
			Database db(Database::File::Accounts);
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
			Database db(Database::File::Accounts, Database::Flag::Read_write);
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
				check_password(old, acc);
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
            sha512_password_hash hash;
            hash.num_rounds(pamh.num_rounds());
			acc.set_password(hash(new_password));
			db.set_password(acc);
			pamh.debug("successfully changed password for user \"%s\"", user);
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
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	return PAM_SUCCESS;
}

int pam_sm_close_session(
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	return PAM_SUCCESS;
}
// }}}
