#ifndef GGG_PAM_PAM_HANDLE_HH
#define GGG_PAM_PAM_HANDLE_HH

#include <unistd.h>
#include <syslog.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>

#include <unistdx/net/socket_address>

#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/pam/call.hh>
#include <ggg/pam/conversation.hh>
#include <ggg/pam/handle.hh>

namespace ggg {

    constexpr const double default_min_entropy = 30.0;

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
        double _minentropy = default_min_entropy;
        sys::socket_address _server_socket_address{GGG_BIND_ADDRESS};

    public:

        inline
        pam_handle(::pam_handle_t* pamh, int argc, const char** argv):
        pam::handle(pamh) {
            this->parse_args(argc, argv);
        }

        inline bool debug() const noexcept { return this->_debug; }
        inline double min_entropy() const noexcept { return this->_minentropy; }

        inline const sys::socket_address& server() const noexcept {
            return this->_server_socket_address;
        }

        account* get_account();
        void set_account(const ggg::account& acc);
        Database* get_database();
        void close_connection();

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
