#ifndef PAM_PAM_HANDLE_HH
#define PAM_PAM_HANDLE_HH

#include <unistd.h>
#include <syslog.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/pam/conversation.hh>
#include <ggg/pam/pam_call.hh>

namespace ggg {

	void
	delete_account(pam_handle_t *pamh, void *data, int error_status);

    struct log_message {
        char hostname[HOST_NAME_MAX];
        account::time_point timestamp;
        inline log_message(): timestamp(account::clock_type::now()) {
            ::gethostname(this->hostname, HOST_NAME_MAX);
        }
    };

	class pam_handle {

    private:
        using cleanup_type = void (*)(::pam_handle_t*,void*,int);

    private:
		::pam_handle_t* _pamh = nullptr;
		bool _debug = false;
		/// Minimal password entropy (as computed by zxcvbn library).
		double _minentropy = 30.0;

	public:

		pam_handle() = default;

		inline explicit
		pam_handle(::pam_handle_t* pamh):
		_pamh(pamh)
		{}

		inline
		pam_handle(::pam_handle_t* pamh, int argc, const char** argv):
		_pamh(pamh) {
			this->parse_args(argc, argv);
		}

		inline bool debug() const noexcept { return this->_debug; }
		inline double min_entropy() const noexcept { return this->_minentropy; }
		const char* get_user() const;
		const char* get_password(pam_errc err) const;
		const char* get_old_password() const;
		const account* get_account() const;
		void set_account(const ggg::account& acc);
		Database& get_database();
        const log_message& get_message();

        inline bool
        get_data(const char* key, const void** ptr) const {
            return ::pam_get_data(*this, key, ptr) == PAM_SUCCESS;
        }

        inline void
        set_data(const char* key, void* ptr, cleanup_type cleanup) {
            if (::pam_set_data(*this, key, ptr, cleanup) != PAM_SUCCESS) {
                throw_pam_error(pam_errc::service_error);
            }
        }

		inline void
		set_password_type(const void* word) {
			this->set_item(PAM_AUTHTOK_TYPE, word);
		}

		void set_item(int key, const void* value);
		conversation_ptr get_conversation() const;
		pam_errc handle_error(const std::system_error& e, pam_errc def) const;

		inline pam_errc
		handle_error(const std::exception& e) const {
			pam_syslog(*this, LOG_ERR, "%s", e.what());
			return pam_errc::service_error;
		}

		inline pam_errc
		handle_error(const std::bad_alloc& e) const {
			pam_syslog(*this, LOG_CRIT, "memory allocation error");
			return pam_errc::system_error;
		}

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

		void parse_args(int argc, const char** argv);
		inline operator ::pam_handle_t*() noexcept { return this->_pamh; }
		inline operator ::pam_handle_t*() const noexcept { return this->_pamh; }
		inline operator ::pam_handle_t**() noexcept { return &this->_pamh; }

	private:

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
