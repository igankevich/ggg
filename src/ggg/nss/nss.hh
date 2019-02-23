#ifndef GGG_NSS_NSS_HH
#define GGG_NSS_NSS_HH

#include <nss.h>

#define NSS_NAME(suffix) _nss_ggg_ ## suffix
#define NSS_FUNCTION(suffix) extern "C" enum nss_status NSS_NAME(suffix)
#define NSS_SETENT(db) NSS_FUNCTION(set ## db ## ent)
#define NSS_ENDENT(db) NSS_FUNCTION(end ## db ## ent)
#define NSS_GETENT_R(db) NSS_FUNCTION(get ## db ## ent_r)
#define NSS_GETENTBY_R(db, by) NSS_FUNCTION(get ## db ## by ## _r)

#define NSS_ENUMERATE(db, type, file) \
	NSS_SETENT(db)(void) { return database.open(ggg::Database::File::file); } \
	NSS_ENDENT(db)(void) { return database.close(); } \
	NSS_GETENT_R(db)( \
		type* result, \
		char* buffer, \
		size_t buflen, \
		int* errnop \
	) { \
		return database.get<type>(result, buffer, buflen, errnop); \
	}

#endif // vim:filetype=cpp
