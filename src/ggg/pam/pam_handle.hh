#ifndef PAM_PAM_HANDLE_HH
#define PAM_PAM_HANDLE_HH

#include <syslog.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>

#include "pam_call.hh"
#include <ggg/core/account.hh>
#include <ggg/core/form_type.hh>
#include "conversation.hh"

namespace ggg {

	void
	delete_account(pam_handle_t *pamh, void *data, int error_status);

	class pam_handle {

		::pam_handle_t* _pamh = nullptr;
		bool _debug = false;
		bool _allowregister = false;
		form_type _type = form_type::console;
		unsigned int _nrounds = 0;
		/// Minimal password entropy (as computed by zxcvbn library).
		double _minentropy = 30.0;

	public:

		pam_handle() = default;

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

		inline bool
		allows_registration() const noexcept {
			return this->_allowregister;
		}

		inline double
		min_entropy() const noexcept {
			return this->_minentropy;
		}

		inline std::string
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

		inline void
		set_password_type(const void* word) {
			this->set_item(PAM_AUTHTOK_TYPE, word);
		}

		void
		set_item(int key, const void* value);

		conversation_ptr
		get_conversation() const;

		pam_errc
		handle_error(const std::system_error& e, pam_errc def) const;

		template <class ... Args>
		inline void
		debug(const char* msg, Args ... args) {
			if (this->_debug) {
				pam_syslog(this->_pamh, LOG_DEBUG, msg, args...);
			}
		}

		template <class ... Args>
		inline void
		error(const char* msg, Args ... args) {
			pam_error(this->_pamh, msg, args...);
		}

		template <class ... Args>
		inline void
		info(const char* msg, Args ... args) {
			pam_info(this->_pamh, "%s", msg, args...);
		}

		void
		parse_args(int argc, const char** argv);

		inline operator ::pam_handle_t*() noexcept {
			return this->_pamh;
		}

		inline operator ::pam_handle_t*() const noexcept {
			return this->_pamh;
		}

		inline operator ::pam_handle_t**() noexcept {
			return &this->_pamh;
		}

	private:
		template<class T>
		static const void**
		void_ptr(const T** ptr) {
			return reinterpret_cast<const void**>(ptr);
		}

		inline void
		print_error(const std::system_error& e) const {
			pam_syslog(
				*this,
				e.code().value() == PAM_PERM_DENIED ? LOG_NOTICE : LOG_ERR,
				"%s",
				e.code().message().data()
			);
		}

	};

}

#endif // PAM_PAM_HANDLE_HH
