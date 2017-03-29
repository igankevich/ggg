#include "pwd.hh"
#include "hierarchy.hh"

namespace {
	legion::Hierarchy hierarchy;
}

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, pw) {
	enum nss_status ret;
	hierarchy.open(sys::path(legion::root_directory));
	try {
		hierarchy.read();
		ret = NSS_STATUS_SUCCESS;
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
	}
	return ret;
}

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, pw) {
	hierarchy.clear();
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, pw)(
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, uid)(
	::uid_t uid,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, nam)(
	const char* name,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}
