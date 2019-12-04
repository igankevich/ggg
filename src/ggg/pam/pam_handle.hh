#ifndef GGG_PAM_PAM_HANDLE_HH
#define GGG_PAM_PAM_HANDLE_HH

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

	class pam_handle: public pam::handle {

    public:
        using pam::handle::error;

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
		account* get_account();
		void set_account(const ggg::account& acc);
		Database* get_database();

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

#endif // vim:filetype=cpp
