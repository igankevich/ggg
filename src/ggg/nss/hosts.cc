#include <netdb.h>

#include <ggg/core/host_address.hh>
#include <ggg/core/system_log.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

    inline size_t
    buffer_size(const ip_address& a) noexcept {
        return sizeof(sys::family_type) + a.size();
    }

    inline size_t
    buffer_size(const host_address& h) noexcept {
        return h.name().size() + 1 + buffer_size(h.address());
    }

    void
    copy_to(const host_address& a, struct ::hostent* lhs, char* buffer) {
        Pointer aliases[0];
        Pointer addresses[2];
        addresses[0].ptr = a.address().data();
        addresses[1].ptr = nullptr;
        Buffer buf(buffer);
        lhs->h_name = buf.write(a.name());
        lhs->h_aliases = buf.write(aliases, 0);
        lhs->h_addrtype = static_cast<int>(a.address().family());
        lhs->h_length = static_cast<int>(a.address().size());
        lhs->h_addr_list = buf.write(addresses, 2);
    }

}

using namespace ggg;

namespace {

    using entity_type = ::hostent;
    thread_local NSS_response<host_address,NSS_kernel::Hosts> database;

}

NSS_SETENT(GGG_MODULE_NAME,host)(int) { return database.open(); }
NSS_ENDENT(GGG_MODULE_NAME,host)(void) { return database.close(); }

NSS_GETENT_R(GGG_MODULE_NAME,host)(
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop,
    int* h_errnop
) {
    return database.get<entity_type>(result, buffer, buflen, errnop, h_errnop);
}


NSS_FUNCTION(GGG_MODULE_NAME, gethostbyname2_r)(
    const char* name,
    int af,
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop,
    int* h_errnop
) {
    nss_status ret;
    int err, h_err = 0;
    try {
        NSS_kernel kernel(NSS_kernel::Hosts, NSS_kernel::Get_by_name);
        kernel.name(name);
        kernel.family(static_cast<sys::family_type>(af));
        Client_protocol proto(GGG_CLIENT_CONF);
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<ip_address>();
        if (response.empty()) {
            ret = NSS_STATUS_NOTFOUND;
            err = ENOENT;
            h_err = HOST_NOT_FOUND;
        } else {
            host_address ha(response.front(), name);
            if (buflen < buffer_size(ha)) {
                ret = NSS_STATUS_TRYAGAIN;
                err = ERANGE;
            } else {
                copy_to(ha, result, buffer);
                ret = NSS_STATUS_SUCCESS;
                err = 0;
            }
        }
    } catch (const std::exception& ex) {
        sys::message("%s: %s", __func__, ex.what());
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
        h_err = NO_RECOVERY;
    } catch (...) {
        sys::message("%s: unknown error", __func__);
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
        h_err = NO_RECOVERY;
    }
    *h_errnop = h_err;
    *errnop = err;
    return ret;
}

NSS_FUNCTION(GGG_MODULE_NAME, gethostbyname_r)(
    const char* name,
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop,
    int* h_errnop
) {
    return NSS_NAME(GGG_MODULE_NAME, gethostbyname2_r)(
        name,
        AF_INET,
        result,
        buffer,
        buflen,
        errnop,
        h_errnop
    );
}

NSS_FUNCTION(GGG_MODULE_NAME, gethostbyaddr_r)(
    const void* addr,
    socklen_t len,
    int af,
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop,
    int* h_errnop
) {
    nss_status ret;
    int err, h_err = 0;
    try {
        auto family = static_cast<sys::family_type>(af);
        ip_address address{family};
        if (address.size() != len) {
            ret = NSS_STATUS_NOTFOUND;
            err = ENOENT;
            h_err = HOST_NOT_FOUND;
        } else {
            std::memcpy(address.data(), addr, len);
            NSS_kernel kernel(NSS_kernel::Hosts, NSS_kernel::Get_by_id);
            kernel.address(address);
            kernel.family(family);
            Client_protocol proto(GGG_CLIENT_CONF);
            proto.process(&kernel, Protocol::Command::NSS_kernel);
            const auto& response = kernel.response<std::string>();
            if (response.empty()) {
                ret = NSS_STATUS_NOTFOUND;
                err = ENOENT;
                h_err = HOST_NOT_FOUND;
            } else {
                if (buflen < buffer_size(address)) {
                    ret = NSS_STATUS_TRYAGAIN;
                    err = ERANGE;
                } else {
                    host_address ha(address, response.front());
                    copy_to(ha, result, buffer);
                    ret = NSS_STATUS_SUCCESS;
                    err = 0;
                }
            }
        }
    } catch (const std::exception& ex) {
        sys::message("%s: %s", __func__, ex.what());
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
        h_err = NO_RECOVERY;
    } catch (...) {
        sys::message("%s: unknown error", __func__);
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
        h_err = NO_RECOVERY;
    }
    *h_errnop = h_err;
    *errnop = err;
    return ret;
}
