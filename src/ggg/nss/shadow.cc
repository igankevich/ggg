#include <shadow.h>
#include <stddef.h>

#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/core/days.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/nss.hh>

using namespace ggg;

namespace {

    using entity_type = ::spwd;
    thread_local NSS_response<account,NSS_kernel::Passwd> database;

}

NSS_ENUMERATE_PROTO(GGG_MODULE_NAME, sp, entity_type)

NSS_GETENTBY_R(GGG_MODULE_NAME, sp, nam)(
    const char* name,
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop
) {
    nss_status ret;
    int err;
    try {
        NSS_kernel kernel(NSS_kernel::Shadow, NSS_kernel::Get_by_name);
        kernel.name(name);
        Client_protocol proto;
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<account>();
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
    #if defined(GGG_TEST)
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    #endif
    } catch (...) {
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    }
    *errnop = err;
    return ret;
}
