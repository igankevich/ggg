#include <stdexcept>

#include <security/pam_ext.h>
#include <security/pam_modules.h>
#include <unistd.h>

#include <ggg/bits/macros.hh>
#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/pam/pam_handle.hh>
#include <ggg/sec/argon2.hh>
#include <ggg/sec/password.hh>
#include <ggg/sec/secure_string.hh>
#include <ggg/proto/authentication.hh>
#include <ggg/proto/protocol.hh>
#include <ggg/proto/result.hh>

using pam::throw_pam_error;
using pam::errc;
using pam::call;
using pam::pam_category;

namespace {
    constexpr const char ggg_result[] = "ggg_result";
    constexpr const char ggg_steps[] = "ggg_steps";
}

GGG_VISIBILITY_DEFAULT
int pam_sm_authenticate(
    pam_handle_t* orig,
    int flags,
    int argc,
    const char **argv
) {
    using namespace ggg;
    using ggg::pam_handle;
    errc ret = errc::ignore;
    pam_handle pamh(orig, argc, argv);
    try {
        const char* user = pamh.user();
        pamh.debug("authenticating user \"%s\"", user);
        const char* password = pamh.password(errc::authentication_error);
        PAM_kernel auth(user, password);
        auth.min_entropy(pamh.min_entropy());
        auth.steps(PAM_kernel::Auth | PAM_kernel::Account | PAM_kernel::Open_session);
        auth.service(pamh.get_item(PAM_SERVICE));
        Client_protocol proto;
        auto result = proto.process(&auth, Protocol::Command::PAM_kernel);
        pamh.set_scalar(ggg_steps, auth.steps());
        pamh.set_scalar(ggg_result, result);
        if (result & PAM_kernel::Auth) {
            pamh.debug("successfully authenticated user \"%s\"", user);
            ret = errc::success;
        } else {
            ret = errc::permission_denied;
        }
    } catch (const std::system_error& e) {
        ret = pamh.error(e, errc::authentication_error);
    } catch (const std::bad_alloc& e) {
        ret = pamh.error(e);
    } catch (const std::exception& e) {
        ret = pamh.error(e);
    }
    return std::make_error_condition(ret).value();
}

GGG_VISIBILITY_DEFAULT
int pam_sm_acct_mgmt(
    pam_handle_t* orig,
    int flags,
    int argc,
    const char **argv
) {
    using namespace ggg;
    using ggg::pam_handle;
    errc ret = errc::ignore;
    pam_handle pamh(orig, argc, argv);
    try {
        const char* user = pamh.user();
        pamh.debug("checking account \"%s\"", user);
        Result::Type result = 0;
        sys::u32 steps = 0;
        if (!pamh.get_scalar<sys::u32>(ggg_steps, steps) ||
            !(steps & PAM_kernel::Account) ||
            !pamh.get_scalar<Result::Type>(ggg_result, result)) {
            PAM_kernel auth;
            auth.name(user);
            auth.min_entropy(pamh.min_entropy());
            auth.steps(PAM_kernel::Account | PAM_kernel::Open_session);
            auth.service(pamh.get_item(PAM_SERVICE));
            Client_protocol proto;
            result = proto.process(&auth, Protocol::Command::PAM_kernel);
            pamh.set_scalar(ggg_steps, auth.steps());
            pamh.set_scalar(ggg_result, result);
        }
        if (result & PAM_kernel::Account) {
            pamh.debug("account \"%s\" is valid", user);
            ret = errc::success;
        } else if (result & PAM_kernel::Suspended) {
            pamh.debug("account \"%s\" has been suspended", user);
            ret = errc::permission_denied;
        } else if (result & PAM_kernel::Expired) {
            pamh.debug("account \"%s\" has expired", user);
            ret = errc::account_expired;
        } else if (result & PAM_kernel::Inactive) {
            pamh.debug("account \"%s\" has been inactive for too long", user);
            ret = errc::account_expired;
        } else if (result & PAM_kernel::Password_expired) {
            pamh.debug("password of account \"%s\" has expired", user);
            ret = errc::new_password_required;
        } else {
            ret = errc::permission_denied;
        }
    } catch (const std::system_error& e) {
        ret = pamh.error(e, errc::permission_denied);
    } catch (const std::bad_alloc& e) {
        ret = pamh.error(e);
    } catch (const std::exception& e) {
        ret = pamh.error(e);
    }
    return std::make_error_condition(ret).value();
}

GGG_VISIBILITY_DEFAULT
int pam_sm_chauthtok(
    pam_handle_t* orig,
    int flags,
    int argc,
    const char **argv
) {
    errc ret = errc::ignore;
    ggg::pam_handle pamh(orig, argc, argv);
    if (flags & PAM_PRELIM_CHECK) {
        ret = errc::success;
    } else if (flags & PAM_UPDATE_AUTHTOK) {
        using namespace ggg;
        try {
            pamh.password_type("GGG");
            const char* user = pamh.user();
            pamh.debug("changing password for user \"%s\"", user);
            PAM_kernel auth;
            auth.name(user);
            auth.min_entropy(pamh.min_entropy());
            if (::getuid() != 0) { auth.old_password(pamh.old_password()); }
            auth.password(pamh.password(errc::authtok_error));
            auth.steps(PAM_kernel::Password);
            auth.service(pamh.get_item(PAM_SERVICE));
            Client_protocol proto;
            auto result = proto.process(&auth, Protocol::Command::PAM_kernel);
            if (result & PAM_kernel::Password) {
                pamh.debug("successfully changed password for user \"%s\"", user);
                ret = errc::success;
            } else if (result & PAM_kernel::Suspended) {
                pamh.debug("account \"%s\" has been suspended", user);
                ret = errc::permission_denied;
            } else if (result & PAM_kernel::Expired) {
                pamh.debug("account \"%s\" has expired", user);
                ret = errc::account_expired;
            } else if (result & PAM_kernel::Inactive) {
                pamh.debug("account \"%s\" has been inactive for too long", user);
                ret = errc::account_expired;
            } else {
                ret = errc::authtok_error;
            }
        } catch (const std::system_error& e) {
            ret = pamh.error(e, errc::authtok_error);
        } catch (const std::bad_alloc& e) {
            ret = pamh.error(e);
        } catch (const std::exception& e) {
            ret = pamh.error(e);
        }
    }
    return std::make_error_condition(ret).value();
}

GGG_VISIBILITY_DEFAULT
int pam_sm_setcred(
    pam_handle_t *pamh,
    int flags,
    int argc,
    const char **argv
) {
    return PAM_SUCCESS;
}

GGG_VISIBILITY_DEFAULT
int pam_sm_open_session(
    pam_handle_t* orig,
    int flags,
    int argc,
    const char **argv
) {
    using namespace ggg;
    using ggg::pam_handle;
    errc ret = errc::ignore;
    ggg::pam_handle pamh(orig, argc, argv);
    try {
        const char* user = pamh.user();
        Result::Type result = 0;
        sys::u32 steps = 0;
        if (!pamh.get_scalar<sys::u32>(ggg_steps, steps) ||
            !(steps & PAM_kernel::Open_session) ||
            !pamh.get_scalar<Result::Type>(ggg_result, result)) {
            PAM_kernel auth;
            auth.name(user);
            auth.steps(PAM_kernel::Open_session);
            auth.service(pamh.get_item(PAM_SERVICE));
            Client_protocol proto;
            result = proto.process(&auth, Protocol::Command::PAM_kernel);
            pamh.set_scalar(ggg_steps, auth.steps());
            pamh.set_scalar(ggg_result, result);
        }
        if (result & PAM_kernel::Open_session) {
            ret = pam::errc::success;
        } else {
            ret = pam::errc::service_error;
        }
    } catch (const std::system_error& e) {
        ret = pamh.error(e, errc::service_error);
    } catch (const std::bad_alloc& e) {
        ret = pamh.error(e);
    } catch (const std::exception& e) {
        ret = pamh.error(e);
    }
    return std::make_error_condition(ret).value();
}

GGG_VISIBILITY_DEFAULT
int pam_sm_close_session(
    pam_handle_t* orig,
    int flags,
    int argc,
    const char **argv
) {
    using namespace ggg;
    using ggg::pam_handle;
    errc ret = errc::ignore;
    ggg::pam_handle pamh(orig, argc, argv);
    try {
        const char* user = pamh.user();
        Result::Type result = 0;
        sys::u32 steps = 0;
        if (!pamh.get_scalar<sys::u32>(ggg_steps, steps) ||
            !(steps & PAM_kernel::Close_session) ||
            !pamh.get_scalar<Result::Type>(ggg_result, result)) {
            PAM_kernel auth;
            auth.name(user);
            auth.steps(PAM_kernel::Close_session);
            auth.service(pamh.get_item(PAM_SERVICE));
            Client_protocol proto;
            result = proto.process(&auth, Protocol::Command::PAM_kernel);
        }
        if (result & PAM_kernel::Close_session) {
            ret = pam::errc::success;
        } else {
            ret = pam::errc::service_error;
        }
    } catch (const std::system_error& e) {
        ret = pamh.error(e, errc::service_error);
    } catch (const std::bad_alloc& e) {
        ret = pamh.error(e);
    } catch (const std::exception& e) {
        ret = pamh.error(e);
    }
    return std::make_error_condition(ret).value();
}
