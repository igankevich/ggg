#ifndef PAM_PAM_HANDLE_HH
#define PAM_PAM_HANDLE_HH

#include <syslog.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include "pam_call.hh"
#include "account.hh"

namespace ggg {

	void
	delete_account(pam_handle_t *pamh, void *data, int error_status);

	class pam_handle {

		::pam_handle_t* _pamh;
		bool _debug = false;
		unsigned int _nrounds = 0;

	public:
		inline explicit
		pam_handle(::pam_handle_t* pamh):
		_pamh(pamh)
		{}

		inline unsigned int
		num_rounds() const noexcept {
			return this->_nrounds;
		}

		inline bool
		debug() const noexcept {
			return this->_debug;
		}

		inline secure_string
		password_id() const noexcept {
			return "6";
		}

		const char*
		get_user() const;

		const char*
		get_password(pam_errc err) const;

		const char*
		get_old_password() const;

		const account*
		get_account() const;

		void
		set_account(const ggg::account& acc);

		pam_errc
		handle_error(const std::system_error& e, pam_errc def) const;

		void
		parse_args(int argc, const char** argv);

		inline operator ::pam_handle_t*() noexcept {
			return _pamh;
		}

		inline operator ::pam_handle_t*() const noexcept {
			return _pamh;
		}

	private:
		template<class T>
		static const void**
		void_ptr(const T** ptr) {
			return reinterpret_cast<const void**>(ptr);
		}

		void
		print_error(const std::system_error& e) const {
			pam_syslog(
				*this,
				e.code().value() == PAM_PERM_DENIED ? LOG_NOTICE : LOG_ERR,
				e.code().message().data()
			);
		}

	};

}

#endif // PAM_PAM_HANDLE_HH
