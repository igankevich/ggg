#ifndef PAM_PAM_ERRC_HH
#define PAM_PAM_ERRC_HH

#include <security/pam_modules.h>
#include <system_error>

namespace ggg {

	enum struct pam_errc: int {
		success=PAM_SUCCESS,
		open_error=PAM_OPEN_ERR,
		symbol_not_found=PAM_SYMBOL_ERR,
		service_error=PAM_SERVICE_ERR,
		system_error=PAM_SYSTEM_ERR,
		buf_error=PAM_BUF_ERR,
		permission_denied=PAM_PERM_DENIED,
		auth_error=PAM_AUTH_ERR,
		insufficient_credentials=PAM_CRED_INSUFFICIENT,
		authinfo_unavail=PAM_AUTHINFO_UNAVAIL,
		unknown_user=PAM_USER_UNKNOWN,
		max_tries=PAM_MAXTRIES,
		new_password_required=PAM_NEW_AUTHTOK_REQD,
		account_expired=PAM_ACCT_EXPIRED,
		session_error=PAM_SESSION_ERR,
		no_credentials=PAM_CRED_UNAVAIL,
		credentials_expired=PAM_CRED_EXPIRED,
		credentials_error=PAM_CRED_ERR,
		no_module_data=PAM_NO_MODULE_DATA,
		conversation_error=PAM_CONV_ERR,
		authtok_error=PAM_AUTHTOK_ERR,
		authtok_recovery_error=PAM_AUTHTOK_RECOVERY_ERR,
		authtok_lock_busy=PAM_AUTHTOK_LOCK_BUSY,
		authtok_disable_aging=PAM_AUTHTOK_DISABLE_AGING,
		try_again=PAM_TRY_AGAIN,
		ignore=PAM_IGNORE,
		abort=PAM_ABORT,
		authtok_expired=PAM_AUTHTOK_EXPIRED,
		unknown_module=PAM_MODULE_UNKNOWN,
		bad_item=PAM_BAD_ITEM,
		try_conversation_again=PAM_CONV_AGAIN,
		incomplete=PAM_INCOMPLETE,
	};

}

namespace std {

	template<>
	struct is_error_condition_enum<ggg::pam_errc>: true_type {};

	inline error_condition
	make_error_condition(ggg::pam_errc e) noexcept;

}

#endif // PAM_PAM_ERRC_HH
