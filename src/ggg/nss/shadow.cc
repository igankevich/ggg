#include <ggg/config.hh>
#include <ggg/nss/hierarchy_instance.hh>
#include <ggg/nss/shadow.hh>

using ggg::database;

namespace {

	typedef sqlite::rstream_iterator<ggg::account> account_iterator;
	ggg::Database::row_stream_t rstr;
	account_iterator first;

	inline void
	init() {
		if (!database.is_open()) {
			database.open(ggg::Database::File::Entities);
			rstr = database.accounts();
			first = account_iterator(rstr);
		}
	}

}

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, sp) {
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

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, sp) {
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

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, sp)(
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	enum nss_status ret = NSS_STATUS_NOTFOUND;
	try {
		account_iterator last;
		if (first == last) {
			ret = NSS_STATUS_NOTFOUND;
			*errnop = ENOENT;
		} else if (buflen < first->buffer_size()) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			first->copy_to(result, buffer);
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

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, sp, nam)(
	const char* name,
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	enum nss_status ret;
	try {
		ggg::Database db(ggg::Database::File::Accounts);
		auto rstr = db.find_account(name);
		account_iterator first(rstr), last;
		if (first == last) {
			ret = NSS_STATUS_NOTFOUND;
			*errnop = ENOENT;
		} else if (buflen < first->buffer_size()) {
			ret = NSS_STATUS_TRYAGAIN;
			*errnop = ERANGE;
		} else {
			first->copy_to(result, buffer);
			ret = NSS_STATUS_SUCCESS;
			*errnop = 0;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		errno = ENOENT;
	}
	return ret;
}
