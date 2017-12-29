#include "pwd.hh"
#include "hierarchy_instance.hh"
#include <iostream>

using ggg::hierarchy;

namespace {

	ggg::Hierarchy::iterator first, last;
	volatile bool initialised = false;

	void
	init(bool force=false) {
		if (!initialised) {
			hierarchy.open(GGG_ENT_ROOT);
			first = hierarchy.begin();
			last = hierarchy.end();
			initialised = true;
		}
	}

}

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, pw) {
	enum nss_status ret;
	try {
		init();
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
	initialised = false;
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, pw)(
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret = NSS_STATUS_NOTFOUND;
	int err = 0;
	if (first != last) {
		if (buflen < buffer_size(*first)) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
		} else {
			copy_to(*first, result, buffer);
			++first;
			ret = NSS_STATUS_SUCCESS;
		}
	}
	*errnop = err;
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
	try {
		ggg::Hierarchy h(GGG_ENT_ROOT);
		auto it = h.find_by_uid(uid);
		if (it != h.end()) {
			if (buflen < buffer_size(*it)) {
				ret = NSS_STATUS_TRYAGAIN;
				*errnop = ERANGE;
			} else {
				copy_to(*it, result, buffer);
				ret = NSS_STATUS_SUCCESS;
				*errnop = 0;
			}
		} else {
			ret = NSS_STATUS_NOTFOUND;
			*errnop = ENOENT;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		errno = ENOENT;
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
	try {
		ggg::Hierarchy h(GGG_ENT_ROOT);
		auto it = h.find_by_name(name);
		if (it != h.end()) {
			if (buflen < buffer_size(*it)) {
				ret = NSS_STATUS_TRYAGAIN;
				*errnop = ERANGE;
			} else {
				copy_to(*it, result, buffer);
				ret = NSS_STATUS_SUCCESS;
				*errnop = 0;
			}
		} else {
			ret = NSS_STATUS_NOTFOUND;
			*errnop = ENOENT;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		errno = ENOENT;
	}
	return ret;
}
