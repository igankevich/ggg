#include <unistd.h>
#include <crypt.h>
#include <fstream>
#include <random>
#include <memory>
#include <locale>
#include <cstdio>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <syslog.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include "config.hh"
#include "hierarchy_instance.hh"
#include "core/account.hh"
#include "sec/secure_string.hh"
#include "sec/secure_sstream.hh"
#include "pam_handle.hh"
#include <stdx/random.hh>
#include "control/account_control.hh"

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

	ggg::Account_control all_accounts;

	ggg::account
	find_account(const char* user) {
		ggg::Account_control::iterator result = all_accounts.find(user);
		if (result == all_accounts.end()) {
			throw_pam_error(ggg::pam_errc::unknown_user);
		}
		return *result;
	}

	ggg::secure_string
	encrypt(const char* password, const ggg::secure_string& prefix) {
		ggg::secure_allocator<crypt_data> alloc;
		std::unique_ptr<crypt_data> pdata(alloc.allocate(1));
		char* encrypted = ::crypt_r(
			password,
			prefix.data(),
			pdata.get()
		);
		if (!encrypted) {
			throw std::system_error(errno, std::system_category());
		}
		ggg::secure_string result(encrypted);
		size_t n = ggg::account::string::traits_type::length(encrypted);
		ggg::shred(encrypted, n);
		return result;
	}

	ggg::secure_string
	generate_salt() {
		ggg::secure_string salt;
		std::random_device prng;
		stdx::adapt_engine<std::random_device,char> engine(prng);
		int i = 0;
		while (i < GGG_SALT_LENGTH) {
			const char ch = engine();
			if (std::isgraph(ch)
				&& ch != ggg::account::separator
				&& ch != ggg::account::delimiter)
			{
				salt.push_back(ch);
				++i;
			}
		}
		return salt;
	}

	void
	check_password(const char* password, const ggg::account& acc) {
		ggg::secure_string encrypted = encrypt(
			password,
			acc.password_prefix()
		);
		if (acc.password() != encrypted) {
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
	pam_errc ret = pam_errc::ignore;
	ggg::pam_handle pamh(orig);
	pamh.parse_args(argc, argv);
	try {
		const char* user = pamh.get_user();
		pamh.debug("authenticating user \"%s\"", user);
		const char* password = pamh.get_password(pam_errc::auth_error);
		ggg::account acc = find_account(user);
		check_password(password, acc);
		pamh.set_account(acc);
		pamh.debug("successfully authenticated user \"%s\"", user);
		ret = pam_errc::success;
	} catch (const std::system_error& e) {
		ret = pamh.handle_error(e, pam_errc::auth_error);
	} catch (const std::bad_alloc& e) {
		ret = pam_errc::auth_error;
		pam_syslog(pamh, LOG_CRIT, "memory allocation error");
	}
	return std::make_error_condition(ret).value();
}

int pam_sm_acct_mgmt(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
	ggg::pam_handle pamh(orig);
	pam_errc ret = pam_errc::ignore;
	pamh.parse_args(argc, argv);
	try {
		const ggg::account* acc = pamh.get_account();
		const char* user = acc->login().c_str();
		pamh.debug("checking account \"%s\"", user);
		const ggg::account::time_point now = ggg::account::clock_type::now();
		if (acc->has_expired(now)) {
			pamh.debug(
				"account \"%s\" has expired %ld day(s) ago",
				user, num_days_since_expired(acc, now)
			);
			throw_pam_error(pam_errc::account_expired);
		}
		if (acc->password_has_expired(now)) {
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
		ret = pam_errc::authtok_error;
		pam_syslog(pamh, LOG_CRIT, "memory allocation error");
	}
	return std::make_error_condition(ret).value();
}

int pam_sm_chauthtok(
	pam_handle_t* orig,
	int flags,
	int argc,
	const char **argv
) {
	ggg::pam_handle pamh(orig);
	pam_errc ret = pam_errc::ignore;
	pamh.parse_args(argc, argv);
	if (flags & PAM_PRELIM_CHECK) {
		ret = pam_errc::success;
	} else if (flags & PAM_UPDATE_AUTHTOK) {
		try {
			const char* user = pamh.get_user();
			pamh.debug("changing password for user \"%s\"", user);
			ggg::account acc = find_account(user);
			const ggg::account::time_point now = ggg::account::clock_type::now();
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
			if (::getuid() != 0) {
				const char* old = pamh.get_old_password();
				check_password(old, acc);
			}
			const char* new_password = pamh.get_password(pam_errc::authtok_error);
			ggg::secure_string encrypted = encrypt(
				new_password,
				acc.password_prefix(
					generate_salt(),
					pamh.password_id(),
					pamh.num_rounds()
				)
			);
			acc.set_password(encrypted);
			all_accounts.update(acc);
			pamh.debug("successfully changed password for user \"%s\"", user);
			ret = pam_errc::success;
		} catch (const std::system_error& e) {
			ret = pamh.handle_error(e, pam_errc::authtok_error);
		} catch (const std::bad_alloc& e) {
			ret = pam_errc::authtok_error;
			pam_syslog(pamh, LOG_CRIT, "memory allocation error");
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
