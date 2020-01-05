#ifndef GGG_PAM_HANDLE_HH
#define GGG_PAM_HANDLE_HH

#include <syslog.h>
#include <security/pam_appl.h>
#include <security/pam_ext.h>
#include <security/pam_modules.h>

#include <stdexcept>

#include <ggg/pam/call.hh>
#include <ggg/pam/conversation.hh>
#include <ggg/pam/errc.hh>

namespace pam {

    class handle {

    private:
        using handle_type = ::pam_handle;
        using cleanup_type = void (*)(handle_type*,void*,int);

    private:
        handle_type* _handle = nullptr;

    public:

        handle() = default;
        inline explicit handle(handle_type* h): _handle(h) {}

        inline void
        start(const char* service, const char* user,
              conversation& converse) {
            call(::pam_start(service, user, &converse, *this));
        }

        inline void authenticate(int flags=0) { call(::pam_authenticate(*this,flags)); }
        inline void change_password(int flags=0) { call(::pam_chauthtok(*this,flags)); }
        inline void verify_account(int flags=0) { call(::pam_acct_mgmt(*this,flags)); }
        inline void end(int status=0) { call(::pam_end(*this,status)); }
        inline void open_session(int flags=0) { call(::pam_open_session(*this,flags)); }
        inline void close_session(int flags=0) { call(::pam_close_session(*this,flags)); }

        inline const char*
        user() {
            const char* user = nullptr;
            if (::pam_get_user(*this, &user, nullptr) != PAM_SUCCESS) {
                throw_pam_error(errc::authentication_error);
            }
            return user;
        }

        inline const char*
        password(errc err) {
            const char* pwd = nullptr;
            if (::pam_get_authtok(*this,PAM_AUTHTOK,&pwd,nullptr) != PAM_SUCCESS) {
                throw_pam_error(err);
            }
            return pwd;
        }

        inline const char*
        old_password() {
            const char* pwd = nullptr;
            if (::pam_get_authtok(*this,PAM_OLDAUTHTOK,&pwd,nullptr) != PAM_SUCCESS) {
                throw_pam_error(errc::authtok_recovery_error);
            }
            return pwd;
        }

        inline bool
        get_data(const char* key, const void** ptr) const {
            return ::pam_get_data(*this, key, ptr) == PAM_SUCCESS;
        }

        inline void
        set_data(const char* key, void* ptr, cleanup_type cleanup) {
            if (::pam_set_data(*this, key, ptr, cleanup) != PAM_SUCCESS) {
                throw_pam_error(errc::service_error);
            }
        }

        template <class T>
        typename std::enable_if<sizeof(T)<=sizeof(void*),bool>::type
        get_scalar(const char* key, T& value) {
            std::intptr_t tmp = 0;
            bool ret = this->get_data(key, reinterpret_cast<const void**>(&tmp));
            value = static_cast<T>(tmp);
            return ret;
        }

        template <class T>
        typename std::enable_if<sizeof(T)<=sizeof(void*),void>::type
        set_scalar(const char* key, T value) {
            std::intptr_t tmp = static_cast<std::intptr_t>(value);
            this->set_data(key, reinterpret_cast<void*>(tmp), nullptr);
        }

        inline void
        set_item(int key, const void* value) {
            call(::pam_set_item(*this, key, value));
        }

        inline void
        password_type(const void* word) {
            this->set_item(PAM_AUTHTOK_TYPE, word);
        }

        template <class T>
        inline int
        get_item(int type, T** ptr) const {
            auto cptr = const_cast<const T**>(ptr);
            return ::pam_get_item(*this, type, reinterpret_cast<const void**>(cptr));
        }

        inline std::string
        get_item(int type) const {
            const char* value = nullptr;
            if (PAM_SUCCESS == this->get_item(PAM_SERVICE, &value) && value) {
                return value;
            }
            return "";
        }

        inline conversation_ptr
        get_conversation() const {
            conversation* ptr = nullptr;
            int ret = get_item(PAM_CONV, &ptr);
            if (ret != PAM_SUCCESS) {
                throw_pam_error(errc(ret));
            }
            if (!ptr || !ptr->conv) {
                throw_pam_error(errc::conversation_error);
            }
            return conversation_ptr(ptr);
        }

        errc error(const std::system_error& e, errc def) const;

        inline errc
        error(const std::exception& e) const {
            pam_syslog(*this, LOG_ERR, "%s", e.what());
            return errc::service_error;
        }

        inline errc
        error(const std::bad_alloc& e) const {
            pam_syslog(*this, LOG_CRIT, "memory allocation error");
            return errc::system_error;
        }

        inline handle_type* get() { return this->_handle; }
        inline const handle_type* get() const { return this->_handle; }
        inline operator handle_type*() { return this->_handle; }
        inline operator handle_type*() const { return this->_handle; }
        inline operator handle_type**() { return &this->_handle; }

    };

}

#endif // vim:filetype=cpp
