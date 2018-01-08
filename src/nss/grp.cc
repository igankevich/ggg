#include "grp.hh"
#include "hierarchy_instance.hh"

using ggg::hierarchy;

namespace {

	ggg::Hierarchy::group_iterator first, last;
	volatile bool initialised = false;

	void
	init() {
		if (!initialised) {
			hierarchy.open(GGG_ENT_ROOT);
			first = hierarchy.group_begin();
			last = hierarchy.group_end();
			initialised = true;
		}
	}

}


NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, gr) {
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

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, gr) {
	hierarchy.clear();
	first = hierarchy.group_begin();
	last = hierarchy.group_end();
	initialised = false;
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, gr)(
	struct ::group* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret = NSS_STATUS_NOTFOUND;
	int err = 0;
	if (first != last) {
		if (buflen < ggg::buffer_size(*first)) {
			err = ERANGE;
			ret = NSS_STATUS_TRYAGAIN;
		} else {
			ggg::copy_to(*first, result, buffer);
			++first;
			ret = NSS_STATUS_SUCCESS;
		}
	}
	*errnop = err;
	return ret;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, gr, gid)(
	::gid_t gid,
	struct ::group* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	try {
		ggg::Hierarchy h(GGG_ENT_ROOT);
		auto it = h.find_group_by_gid(gid);
		if (it != h.group_end()) {
			if (buflen < ggg::buffer_size(*it)) {
				ret = NSS_STATUS_TRYAGAIN;
				*errnop = ERANGE;
			} else {
				ggg::copy_to(*it, result, buffer);
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

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, gr, nam)(
	const char* name,
	struct ::group* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	try {
		ggg::Hierarchy h(GGG_ENT_ROOT);
		auto it = h.find_group_by_name(name);
		if (it != h.group_end()) {
			if (buflen < ggg::buffer_size(*it)) {
				ret = NSS_STATUS_TRYAGAIN;
				*errnop = ERANGE;
			} else {
				ggg::copy_to(*it, result, buffer);
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

/*
NSS_MODULE_FUNCTION_INITGROUPS(MODULE_NAME)(
	const char *user,
	gid_t group,
	long int *start,
	long int *size,
	gid_t **groupsp,
	long int limit,
	int *errnop
) {
	// TODO
	return NSS_STATUS_UNAVAIL;
}
*/
