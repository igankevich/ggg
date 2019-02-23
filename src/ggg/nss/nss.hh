#ifndef GGG_NSS_NSS_HH
#define GGG_NSS_NSS_HH

#include <nss.h>

#define NSS_FUNCTION(suffix) extern "C" enum nss_status _nss_ggg_ ## suffix
#define NSS_SETENT(db) NSS_FUNCTION(set ## db ## ent)(void)
#define NSS_ENDENT(db) NSS_FUNCTION(end ## db ## ent)(void)
#define NSS_GETENT_R(db) NSS_FUNCTION(get ## db ## ent_r)
#define NSS_GETENTBY_R(db, by) NSS_FUNCTION(get ## db ## by ## _r)

#endif // vim:filetype=cpp
