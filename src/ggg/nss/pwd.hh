#ifndef PWD_HH
#define PWD_HH

#include <pwd.h>
#include <stddef.h>

#include <ggg/nss/nss.hh>

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, pw);

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, pw);

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, pw)(
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
);

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, uid)(
	::uid_t uid,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
);

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, nam)(
	const char* name,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
);

#endif // PWD_HH
