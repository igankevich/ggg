#ifndef SHADOW_HH
#define SHADOW_HH

#include <shadow.h>
#include <stddef.h>
#include "nss.hh"

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, sp);

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, sp);

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, sp)(
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
);

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, sp, nam)(
	const char* name,
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
);

#endif // SHADOW_HH
