#include "shadow.hh"

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, sp) {
	enum nss_status ret = NSS_STATUS_SUCCESS;
	return ret;
}

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, sp) {
	enum nss_status ret = NSS_STATUS_SUCCESS;
	return ret;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, sp)(
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	enum nss_status ret = NSS_STATUS_SUCCESS;
	return ret;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, sp, nam)(
	const char* name,
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	enum nss_status ret = NSS_STATUS_SUCCESS;
	return ret;
}
