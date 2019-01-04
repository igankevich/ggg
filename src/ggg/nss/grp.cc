#include <ggg/nss/hierarchy_instance.hh>
#include <ggg/nss/grp.hh>

using ggg::database;

namespace {

	ggg::Database::row_stream_t rstr;
	ggg::group_iterator first;

	inline void
	init() {
		if (!database.is_open()) {
			database.open(GGG_DATABASE_PATH);
			rstr = database.users();
			first = ggg::group_iterator(rstr);
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
	enum nss_status ret;
	try {
		rstr.close();
		database.close();
		ret = NSS_STATUS_SUCCESS;
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		errno = ENOENT;
	}
	return ret;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, gr)(
	struct ::group* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret = NSS_STATUS_NOTFOUND;
	try {
		ggg::group_iterator last;
		if (first == last) {
			ret = NSS_STATUS_NOTFOUND;
			*errnop = ENOENT;
		} else if (buflen < ggg::buffer_size(*first)) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			ggg::copy_to(*first, result, buffer);
			++first;
			ret = NSS_STATUS_SUCCESS;
			*errnop = 0;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		errno = ENOENT;
	}
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
		ggg::Database db(GGG_DATABASE_PATH);
		ggg::group gr;
		if (!db.find_group(gid, gr)) {
			ret = NSS_STATUS_NOTFOUND;
			*errnop = ENOENT;
		} else if (buflen < ggg::buffer_size(gr)) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			ggg::copy_to(gr, result, buffer);
			ret = NSS_STATUS_SUCCESS;
			*errnop = 0;
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
		ggg::Database db(GGG_DATABASE_PATH);
		ggg::group gr;
		if (!db.find_group(name, gr)) {
			ret = NSS_STATUS_NOTFOUND;
			*errnop = ENOENT;
		} else if (buflen < ggg::buffer_size(gr)) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			ggg::copy_to(gr, result, buffer);
			ret = NSS_STATUS_SUCCESS;
			*errnop = 0;
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
