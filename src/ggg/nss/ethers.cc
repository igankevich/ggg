#include <ggg/core/database.hh>
#include <ggg/core/host.hh>
#include <ggg/core/system_log.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/entity_traits.hh>
#include <ggg/nss/etherent.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

    template <>
    size_t
    buffer_size<host>(const host& h) noexcept {
        return h.name().size() + 1 + h.address().size();
    }

    template <>
    void
    copy_to<host>(const host& ent, struct ::etherent* lhs, char* buffer) {
        Buffer buf(buffer);
        lhs->e_name = buf.write(ent.name());
        std::memcpy(
            lhs->e_addr.ether_addr_octet,
            ent.address().data(),
            ent.address().size()
        );
    }

}

using namespace ggg;

NSS_FUNCTION(GGG_MODULE_NAME, gethostton_r)(
    const char* name,
    struct ::etherent* result,
    char* buffer,
    size_t buflen,
    int* errnop
) {
    nss_status ret;
    int err;
    try {
        NSS_kernel kernel(NSS_kernel::Ethers, NSS_kernel::Get_by_name);
        kernel.name(name);
        Client_protocol proto(GGG_CLIENT_CONF);
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<host>();
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
    } catch (const std::exception& ex) {
        sys::message("%s: %s", __func__, ex.what());
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    } catch (...) {
        sys::message("%s: unknown error", __func__);
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    }
    *errnop = err;
    return ret;
}

NSS_FUNCTION(GGG_MODULE_NAME, getntohost_r)(
    const struct ::ether_addr* address,
    struct ::etherent* result,
    char* buffer,
    size_t buflen,
    int* errnop
) {
    nss_status ret;
    int err;
    try {
        NSS_kernel kernel(NSS_kernel::Ethers, NSS_kernel::Get_by_id);
        kernel.ethernet_address(sys::ethernet_address(address->ether_addr_octet));
        Client_protocol proto(GGG_CLIENT_CONF);
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<host>();
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
    } catch (const std::exception& ex) {
        sys::message("%s: %s", __func__, ex.what());
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    } catch (...) {
        sys::message("%s: unknown error", __func__);
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    }
    *errnop = err;
    return ret;
}
