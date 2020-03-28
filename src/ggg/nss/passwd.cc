#include <pwd.h>
#include <stddef.h>

#include <ggg/config.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

    template <>
    struct entity_traits<entity> {

        typedef entity entity_type;
        typedef Database::statement_type stream_type;
        typedef sqlite::row_iterator<entity> iterator;

        static inline stream_type
        all(Database* db) {
            return db->entities();
        }

    };

}

namespace {

    typedef struct ::passwd entity_type;
    ggg::NSS_database<ggg::entity> database;

}

NSS_ENUMERATE(pw, entity_type, Entities)

NSS_GETENTBY_R(pw, uid)(
    ::uid_t uid,
    entity_type* result,
    char* buffer,
    size_t buflen,
    int* errnop
) {
    nss_status ret;
    int err;
    try {
        ggg::Database db(ggg::Database::File::Entities);
        auto rstr = db.find_user(uid);
        ggg::user_iterator first(rstr), last;
        if (first == last) {
            ret = NSS_STATUS_NOTFOUND;
            err = ENOENT;
        } else if (buflen < buffer_size(*first)) {
            ret = NSS_STATUS_TRYAGAIN;
            err = ERANGE;
        } else {
            copy_to(*first, result, buffer);
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
        ggg::Database db(ggg::Database::File::Entities);
        auto rstr = db.find_user(name);
        ggg::user_iterator first(rstr), last;
        if (first == last) {
            ret = NSS_STATUS_NOTFOUND;
            err = ENOENT;
        } else if (buflen < buffer_size(*first)) {
            ret = NSS_STATUS_TRYAGAIN;
            err = ERANGE;
        } else {
            copy_to(*first, result, buffer);
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
