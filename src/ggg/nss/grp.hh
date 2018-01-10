#ifndef GRP_HH
#define GRP_HH

#include <grp.h>
#include <stddef.h>
#include "nss.hh"

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, gr);

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, gr);

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, gr)(
	struct ::group* result,
	char* buffer,
	size_t buflen,
	int* errnop
);

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, gr, gid)(
	::gid_t gid,
	struct ::group* result,
	char* buffer,
	size_t buflen,
	int* errnop
);

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, gr, nam)(
	const char* name,
	struct ::group* result,
	char* buffer,
	size_t buflen,
	int* errnop
);

/*
NSS_MODULE_FUNCTION_INITGROUPS(MODULE_NAME)(
	const char *user,
	gid_t group,
	long int *start,
	long int *size,
	gid_t **groupsp,
	long int limit,
	int *errnop
);
*/

#endif // GRP_HH

