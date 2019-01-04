#include <ggg/nss/hierarchy_instance.hh>
#include <ggg/nss/pwd.hh>

using ggg::database;

namespace {

	ggg::Database::row_stream_t rstr;
	ggg::entity current;

	inline void
	init() {
		if (!database.is_open()) {
			database.open(GGG_DATABASE_PATH);
			rstr = database.users();
			rstr >> current;
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

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, pw)(
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret = NSS_STATUS_NOTFOUND;
	try {
		if (!rstr.good()) {
			ret = NSS_STATUS_NOTFOUND;
			*errnop = ENOENT;
		} else if (buflen < buffer_size(current)) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			copy_to(current, result, buffer);
			rstr >> current;
			ret = NSS_STATUS_SUCCESS;
			*errnop = 0;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		errno = ENOENT;
	}
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
		ggg::Database db(GGG_DATABASE_PATH);
		auto rstr = db.find_user(uid);
		ggg::user_iterator first(rstr), last;
		if (first != last) {
			if (buflen < buffer_size(*first)) {
				ret = NSS_STATUS_TRYAGAIN;
				*errnop = ERANGE;
			} else {
				copy_to(*first, result, buffer);
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
		ggg::Database db(GGG_DATABASE_PATH);
		auto rstr = db.find_user(name);
		ggg::user_iterator first(rstr), last;
		if (first != last) {
			if (buflen < buffer_size(*first)) {
				ret = NSS_STATUS_TRYAGAIN;
				*errnop = ERANGE;
			} else {
				copy_to(*first, result, buffer);
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
