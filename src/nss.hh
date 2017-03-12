#ifndef NSS_HH
#define NSS_HH

#include <nss.h>
#include "config.hh"

#define NSS_MODULE_FUNCTION(module, prefix, db, suffix) \
	extern "C" \
	enum nss_status \
	_nss_##module##_##prefix##db##suffix

#define NSS_MODULE_FUNCTION_SETENT(module, db) \
	NSS_MODULE_FUNCTION(module, set, pw, ent)(void)
#define NSS_MODULE_FUNCTION_ENDENT(module, db) \
	NSS_MODULE_FUNCTION(module, end, pw, ent)(void)
#define NSS_MODULE_FUNCTION_GETENT_R(module, db) \
	NSS_MODULE_FUNCTION(module, get, pw, ent_r)
#define NSS_MODULE_FUNCTION_GETENTBY_R(module, db, by) \
	NSS_MODULE_FUNCTION(module, get, pw, by##_r)


#endif // NSS_HH
