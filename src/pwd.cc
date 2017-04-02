#include "pwd.hh"
#include "hierarchy.hh"

namespace {
	legion::Hierarchy hierarchy(sys::path(legion::root_directory));
	legion::Hierarchy::iterator first, last;
}

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, pw) {
	enum nss_status ret;
	try {
		first = hierarchy.begin();
		last = hierarchy.end();
		ret = NSS_STATUS_SUCCESS;
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		errno = ENOENT;
	}
	return ret;
}

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, pw) {
	hierarchy.clear();
	first = hierarchy.begin();
	last = hierarchy.end();
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, pw)(
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret = NSS_STATUS_NOTFOUND;
	if (first != last) {
		if (buflen < first->buffer_size()) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			first->copy_to(result, buffer);
			++first;
			ret = NSS_STATUS_SUCCESS;
		}
	}
	*errnop = 0;
	return ret;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, uid)(
	::uid_t uid,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	auto it = hierarchy.find_by_uid(uid);
	if (it != last) {
		if (buflen < it->buffer_size()) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			it->copy_to(result, buffer);
			ret = NSS_STATUS_SUCCESS;
			*errnop = 0;
		}
	} else {
		ret = NSS_STATUS_NOTFOUND;
		*errnop = ENOENT;
	}
	return ret;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, nam)(
	const char* name,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	auto it = hierarchy.find_by_name(name);
	if (it != last) {
		if (buflen < it->buffer_size()) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			it->copy_to(result, buffer);
			ret = NSS_STATUS_SUCCESS;
			*errnop = 0;
		}
	} else {
		ret = NSS_STATUS_NOTFOUND;
		*errnop = ENOENT;
	}
	return ret;
}
