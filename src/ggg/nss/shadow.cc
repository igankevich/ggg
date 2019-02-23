#include <shadow.h>
#include <stddef.h>

#include <ggg/config.hh>
#include <ggg/nss/hierarchy_instance.hh>
#include <ggg/nss/nss.hh>

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

NSS_SETENT(sp) {
	nss_status ret;
	try {
		init();
		ret = NSS_STATUS_SUCCESS;
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
	}
	return ret;
}

NSS_ENDENT(sp) {
	nss_status ret;
	try {
		rstr.close();
		database.close();
		ret = NSS_STATUS_SUCCESS;
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
	}
	return ret;
}

NSS_GETENT_R(sp)(
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	int err;
	try {
		account_iterator last;
		if (first == last) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
		} else if (buflen < first->buffer_size()) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
		} else {
			first->copy_to(result, buffer);
			++first;
			ret = NSS_STATUS_SUCCESS;
			err = 0;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		err = ENOENT;
	}
	*errnop = err;
	return ret;
}

NSS_GETENTBY_R(sp, nam)(
	const char* name,
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	int err;
	try {
		ggg::Database db(ggg::Database::File::Accounts);
		auto rstr = db.find_account(name);
		account_iterator first(rstr), last;
		if (first == last) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
		} else if (buflen < first->buffer_size()) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
		} else {
			first->copy_to(result, buffer);
			ret = NSS_STATUS_SUCCESS;
			err = 0;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		err = ENOENT;
	}
	*errnop = err;
	return ret;
}
