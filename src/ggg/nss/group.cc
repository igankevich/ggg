#include <cstdlib>
#include <grp.h>
#include <limits>
#include <stddef.h>

#include <ggg/core/entity.hh>
#include <ggg/core/group.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/nss.hh>

#if defined(GGG_DEBUG_INITGROUPS)
#include <iostream>
#endif

using entity_type = ::group;

using namespace ggg;

namespace {

    NSS_response<::ggg::group,NSS_kernel::Group> database;

}

NSS_ENUMERATE_PROTO(gr, entity_type)

NSS_GETENTBY_R(gr, gid)(
    ::gid_t gid,
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop
) {
    nss_status ret;
    int err;
    try {
        NSS_kernel kernel(NSS_kernel::Group, NSS_kernel::Get_by_id);
        kernel.gid(gid);
        Client_protocol proto;
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<::ggg::group>();
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

NSS_GETENTBY_R(gr, nam)(
    const char* name,
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop
) {
    nss_status ret;
    int err;
    try {
        NSS_kernel kernel(NSS_kernel::Group, NSS_kernel::Get_by_name);
        kernel.name(name);
        Client_protocol proto;
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<::ggg::group>();
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

NSS_FUNCTION(initgroups_dyn)(
    const char* user,
    ::gid_t group_id,
    long int* start_ptr,
    long int* size_ptr,
    ::gid_t** groupsp,
    long int max_size,
    int* errnop
) {
    const long int default_size = 4096 / sizeof(::gid_t);
    nss_status ret = NSS_STATUS_SUCCESS;
    int err = 0;
    if (max_size <= 0) {
        max_size = std::numeric_limits<long int>::max();
    }
    auto& start = *start_ptr;
    auto& size = *size_ptr;
    try {
        #if defined(GGG_DEBUG_INITGROUPS)
        std::clog << "user=" << user << std::endl;
        std::clog << "group_id=" << group_id << std::endl;
        std::clog << "start=" << start << std::endl;
        std::clog << "size=" << size << std::endl;
        std::clog << "max_size=" << max_size << std::endl;
        #endif
        NSS_kernel kernel(NSS_kernel::Group, NSS_kernel::Init_groups);
        kernel.name(user);
        Client_protocol proto;
        proto.process(&kernel, Protocol::Command::NSS_kernel);
        const auto& response = kernel.response<entity>();
        for (const auto& ent : response) {
            if (ent.id() == group_id) { continue; }
            if (start >= max_size) {
                ret = NSS_STATUS_TRYAGAIN;
                err = ERANGE;
                break;
            }
            if (start >= size) {
                auto newsize = (size == 0) ? default_size : (size*2);
                auto newgroups = std::realloc(*groupsp, newsize*sizeof(gid_t));
                if (!newgroups) {
                    ret = NSS_STATUS_TRYAGAIN;
                    err = ERANGE;
                    break;
                }
                size = newsize;
                *groupsp = reinterpret_cast<gid_t*>(newgroups);
            }
            (*groupsp)[start++] = ent.id();
        }
    } catch (...) {
        ret = NSS_STATUS_UNAVAIL;
        err = ENOENT;
    }
    *errnop = err;
    return ret;
}
