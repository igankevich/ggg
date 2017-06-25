#include <unistd.h>
#include <crypt.h>
#include <fstream>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include "config.hh"
#include "hierarchy_instance.hh"
#include "pam_call.hh"
#include "account.hh"
#include "secure_allocator.hh"

using ggg::pam::throw_pam_error;
using ggg::pam_errc;
using ggg::pam::call;

namespace {

	ggg::account
	find_account(const char* user) {
		std::ifstream accfile(sys::path(GGG_ROOT, "shadow"));
		std::istream_iterator<ggg::account> last;
		auto it = std::find_if(
			std::istream_iterator<ggg::account>(accfile),
			last,
			[user] (const ggg::account& rhs) {
				return rhs.login().compare(user) == 0;
			}
		);
		if (it == last) {
			throw_pam_error(ggg::pam_errc::unknown_user);
		}
		return *it;
	}

}

int pam_sm_authenticate(
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	pam_info(pamh, __func__);
	struct ::crypt_data data{};
	char* encrypted = nullptr;
	int ret = 0;
	try {
		const char* user = nullptr;
		const char* password = nullptr;
		call(pam_get_user(pamh, &user, nullptr));
		call(pam_get_authtok(pamh, PAM_AUTHTOK, &password, nullptr));
		ggg::account acc = find_account(user);

		encrypted = ::crypt_r(password, acc.password_prefix().data(), &data);
		if (!encrypted) {
			throw std::system_error(errno, std::system_category());
		}
		if (acc.password() != encrypted) {
			throw_pam_error(pam_errc::permission_denied);
		}

		pam_info(pamh, "User: \"%s\"", user);
		pam_info(pamh, "Password: \"%s\"", password);
		pam_info(pamh, "Id: \"%s\"", acc.password_id().data());
		pam_info(pamh, "Salt: \"%s\"", acc.password_salt().data());
		pam_info(pamh, "Password: \"%s\"", acc.password().data());
		pam_info(pamh, "Encrypted: \"%s\"", encrypted);
	} catch (std::system_error e) {
		ret = int(pam_errc::auth_error);
		pam_error(pamh, "Error: \"%s\"", e.code().message().c_str());
	}
	sys::shred(&data, sizeof(data));
	if (encrypted) {
		size_t n = ggg::account::string::traits_type::length(encrypted);
		sys::shred(encrypted, n);
	}
	return ret;
}

int pam_sm_setcred(
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	pam_info(pamh, __func__);
	return PAM_SUCCESS;
}

int pam_sm_acct_mgmt(
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	pam_info(pamh, __func__);
	return PAM_SUCCESS;
}

int pam_sm_open_session(
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	pam_info(pamh, __func__);
	return PAM_SUCCESS;
}

int pam_sm_close_session(
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	pam_info(pamh, __func__);
	return PAM_SUCCESS;
}

int pam_sm_chauthtok(
	pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv
) {
	pam_info(pamh, __func__);
	return PAM_SUCCESS;
}
