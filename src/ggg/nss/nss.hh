#ifndef NSS_HH
#define NSS_HH

#include <nss.h>

#define NSS_MODULE_FUNCTION(module, prefix, db, suffix) \
	extern "C" \
	enum nss_status \
	_nss_##module##_##prefix##db##suffix

#define NSS_MODULE_FUNCTION_SETENT(module, db) \
	NSS_MODULE_FUNCTION(module, set, db, ent)(void)
#define NSS_MODULE_FUNCTION_ENDENT(module, db) \
	NSS_MODULE_FUNCTION(module, end, db, ent)(void)
#define NSS_MODULE_FUNCTION_GETENT_R(module, db) \
	NSS_MODULE_FUNCTION(module, get, db, ent_r)
#define NSS_MODULE_FUNCTION_GETENTBY_R(module, db, by) \
	NSS_MODULE_FUNCTION(module, get, db, by##_r)
#define NSS_MODULE_FUNCTION_INITGROUPS(module) \
	NSS_MODULE_FUNCTION(module, initgroups, _, dyn)


#endif // NSS_HH
