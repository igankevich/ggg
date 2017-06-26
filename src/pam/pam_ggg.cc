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
#include "account.hh"
#include "secure_string.hh"
#include "pam_handle.hh"
#include <stdx/random.hh>

using ggg::throw_pam_error;
using ggg::pam_errc;
using ggg::pam::call;
using ggg::pam_category;

namespace {

	static constexpr const char separator = '$';

	ggg::account
	find_account(const char* user) {
		typedef std::istream_iterator<ggg::account> iterator;
		std::ifstream shadow;
		iterator it, last;
		try {
			shadow.exceptions(std::ios::badbit | std::ios::failbit);
			shadow.open(GGG_SHADOW);
			it = std::find_if(
				std::istream_iterator<ggg::account>(shadow),
				last,
				[user] (const ggg::account& rhs) {
					return rhs.login().compare(user) == 0;
				}
			);
			shadow.close();
		} catch (...) {
			if (shadow.eof()) {
				throw_pam_error(ggg::pam_errc::unknown_user);
			} else {
				throw std::system_error(
					std::io_errc::stream,
					"unable to read accounts from " GGG_SHADOW
				);
			}
		}
		if (it == last) {
			throw_pam_error(ggg::pam_errc::unknown_user);
		}
		return *it;
	}

	void
	write_account(const ggg::account& acc) {
		typedef std::istream_iterator<ggg::account> in_iterator;
		typedef std::ostream_iterator<ggg::account> out_iterator;
		std::ifstream shadow;
		std::ofstream shadow_new;
		try {
			shadow.exceptions(std::ios::badbit | std::ios::failbit);
			shadow.open(GGG_SHADOW);
			shadow_new.exceptions(std::ios::badbit | std::ios::failbit);
			shadow_new.open(GGG_SHADOW_NEW);
			std::transform(
				in_iterator(shadow),
				in_iterator(),
				out_iterator(shadow_new, "\n"),
				[&acc] (const ggg::account& rhs) {
					return rhs.login() == acc.login() ? acc : rhs;
				}
			);
			shadow.close();
			shadow_new.close();
		} catch (...) {
			if (!shadow.eof()) {
				throw std::system_error(
					std::io_errc::stream,
					"unable to write accounts to " GGG_SHADOW
				);
			}
		}
		int ret = std::rename(GGG_SHADOW_NEW, GGG_SHADOW);
		if (ret == -1) {
			throw std::system_error(errno, std::system_category());
		}
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

	void
	generate_salt(ggg::secure_string& salt) {
		std::random_device prng;
		stdx::adapt_engine<std::random_device,char> engine(prng);
		int i = 0;
		while (i < GGG_SALT_LENGTH) {
			const char ch = engine();
			if (std::isgraph(ch)
				&& ch != separator
				&& ch != ggg::account::delimiter)
			{
				salt.push_back(ch);
				++i;
			}
		}
	}

	ggg::secure_string
	generate_prefix(const ggg::account& acc) {
		ggg::secure_string prefix;
		prefix.push_back(separator);
		prefix.append(acc.password_id());
		prefix.push_back(separator);
		generate_salt(prefix);
		prefix.push_back(separator);
		return prefix;
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
		const char* password = pamh.get_password(pam_errc::auth_error);
		ggg::account acc = find_account(user);
		check_password(password, acc);
		pamh.set_account(acc);
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
		if (acc->has_expired()) {
			throw_pam_error(pam_errc::account_expired);
		}
		if (acc->password_has_expired()) {
			throw_pam_error(pam_errc::new_password_required);
		}
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
			ggg::account acc = find_account(user);
			if ((flags & PAM_CHANGE_EXPIRED_AUTHTOK) && !acc.has_expired()) {
				throw_pam_error(pam_errc::authtok_error);
			}
			if (::getuid() != 0) {
				const char* old = pamh.get_old_password();
				check_password(old, acc);
			}
			const char* new_password = pamh.get_password(pam_errc::authtok_error);
			ggg::secure_string encrypted = encrypt(
				new_password,
				generate_prefix(acc)
			);
			acc.set_password(encrypted);
			write_account(acc);
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
