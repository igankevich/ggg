#include <pwd.h>
#include <stddef.h>

#include <ggg/config.hh>
#include <ggg/core/entity.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/nss.hh>
#include <ggg/proto/protocol.hh>
#include <ggg/proto/selection.hh>

using namespace ggg;

namespace {

    using entity_type = ::passwd;
    thread_local NSS_response<entity,NSS_kernel::Passwd> database;

}

NSS_ENUMERATE_PROTO(pw, entity_type)

NSS_GETENTBY_R(pw, uid)(
    ::uid_t uid,
    ::entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop
) {
    nss_status ret;
    int err;
    try {
        NSS_kernel kernel(NSS_kernel::Passwd, NSS_kernel::Get_by_id);
        kernel.uid(uid);
        Client_protocol proto;
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<entity>();
        if (response.empty()) {
            ret = NSS_STATUS_NOTFOUND;
            err = ENOENT;
        } else if (buflen < buffer_size(response.front())) {
            ret = NSS_STATUS_TRYAGAIN;
            err = ERANGE;
        } else {
            copy_to(response.front(), result, buffer);
            ret = NSS_STATUS_SUCCESS;
            err = 0;
        }
    } catch (...) {
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    }
    *errnop = err;
    return ret;
}

NSS_GETENTBY_R(pw, nam)(
    const char* name,
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop
) {
    nss_status ret;
    int err;
    try {
        NSS_kernel kernel(NSS_kernel::Passwd, NSS_kernel::Get_by_name);
        kernel.name(name);
        Client_protocol proto;
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<entity>();
        if (response.empty()) {
            ret = NSS_STATUS_NOTFOUND;
            err = ENOENT;
        } else if (buflen < buffer_size(response.front())) {
            ret = NSS_STATUS_TRYAGAIN;
            err = ERANGE;
        } else {
            copy_to(response.front(), result, buffer);
            ret = NSS_STATUS_SUCCESS;
            err = 0;
        }
    } catch (...) {
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    }
    *errnop = err;
    return ret;
}
