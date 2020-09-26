#ifndef GGG_NSS_NSS_HH
#define GGG_NSS_NSS_HH

#include <nss.h>

#include <ggg/bits/macros.hh>
#include <ggg/config.hh>

#define NSS_NAME_IMPL(name,suffix) _nss_ ## name ## _ ## suffix
#define NSS_NAME(name,suffix) NSS_NAME_IMPL(name,suffix)
#define NSS_FUNCTION(name,suffix) \
    extern "C" GGG_VISIBILITY_DEFAULT \
    enum nss_status NSS_NAME(name, suffix)

#define NSS_SETENT(name, db) NSS_FUNCTION(name, set ## db ## ent)
#define NSS_ENDENT(name, db) NSS_FUNCTION(name, end ## db ## ent)
#define NSS_GETENT_R(name, db) NSS_FUNCTION(name, get ## db ## ent_r)
#define NSS_GETENTBY_R(name, db, by) NSS_FUNCTION(name, get ## db ## by ## _r)

#define NSS_ENUMERATE(name, db, type, file) \
    NSS_SETENT(name, db)(void) { return database.open(ggg::Database::File::file); } \
    NSS_ENDENT(name, db)(void) { return database.close(); } \
    NSS_GETENT_R(name, db)( \
        type* result, \
        char* buffer, \
        size_t buflen, \
        int* errnop \
    ) { \
        return database.get<type>(result, buffer, buflen, errnop); \
    }

#define NSS_ENUMERATE_PROTO(name, db, type) \
    NSS_SETENT(name, db)(void) { return database.open(); } \
    NSS_ENDENT(name, db)(void) { return database.close(); } \
    NSS_GETENT_R(name, db)( \
        type* result, \
        char* buffer, \
        size_t buflen, \
        int* errnop \
    ) { \
        return database.get<type>(result, buffer, buflen, errnop); \
    }

#endif // vim:filetype=cpp
