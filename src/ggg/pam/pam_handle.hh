#ifndef PAM_PAM_HANDLE_HH
#define PAM_PAM_HANDLE_HH

#include <unistd.h>
#include <syslog.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/pam/call.hh>
#include <ggg/pam/conversation.hh>
#include <ggg/pam/handle.hh>

namespace ggg {

    struct log_message {
        char hostname[HOST_NAME_MAX];
        account::time_point timestamp;
        inline log_message(): timestamp(account::clock_type::now()) {
            ::gethostname(this->hostname, HOST_NAME_MAX);
        }
    };

	class pam_handle: public pam::handle {

    private:
        using cleanup_type = void (*)(::pam_handle_t*,void*,int);
        using errc = pam::errc;
        using conversation_ptr = pam::conversation_ptr;

    private:
		bool _debug = false;
		/// Minimal password entropy (as computed by zxcvbn library).
		double _minentropy = 30.0;

	public:

		inline
		pam_handle(::pam_handle_t* pamh, int argc, const char** argv):
		pam::handle(pamh) {
			this->parse_args(argc, argv);
		}

		inline bool debug() const noexcept { return this->_debug; }
		inline double min_entropy() const noexcept { return this->_minentropy; }
		const account* get_account() const;
		void set_account(const ggg::account& acc);
		Database& get_database();
        const log_message& get_message();

		template <class ... Args>
		inline void
		debug(const char* msg, Args ... args) {
			if (this->_debug) {
				pam_syslog(get(), LOG_DEBUG, msg, args...);
			}
		}

		template <class ... Args>
		inline void
		error(const char* msg, Args ... args) {
			pam_error(get(), msg, args...);
		}

		template <class ... Args>
		inline void
		info(const char* msg, Args ... args) {
			pam_info(get(), "%s", msg, args...);
		}

		void parse_args(int argc, const char** argv);

	};

}

#endif // PAM_PAM_HANDLE_HH
