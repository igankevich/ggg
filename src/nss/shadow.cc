#include "shadow.hh"
#include "core/account.hh"
#include <fstream>
#include <algorithm>
#include <iterator>
#include <sys/bits/check.hh>
#include <sys/path.hh>

#include "config.hh"

namespace {

	std::ifstream accfile;

	void
	ensure_open() {
		if (accfile.is_open()) {
			accfile.clear();
		}
		accfile.open(GGG_SHADOW);
	}

}

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, sp) {
	enum nss_status ret;
	try {
		ensure_open();
		ret = NSS_STATUS_SUCCESS;
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		errno = ENOENT;
	}
	return ret;
}

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, sp) {
	accfile.close();
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, sp)(
	struct ::spwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	enum nss_status ret = NSS_STATUS_NOTFOUND;
	int err = 0;
	ggg::account acc;
	if (accfile >> acc) {
		if (buflen < acc.buffer_size()) {
			err = ERANGE;
			ret = NSS_STATUS_TRYAGAIN;
		} else {
			acc.copy_to(result, buffer);
			ret = NSS_STATUS_SUCCESS;
		}
	}
	*errnop = err;
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
	ensure_open();
	std::istream_iterator<ggg::account> last;
	auto it = std::find_if(
		std::istream_iterator<ggg::account>(accfile),
		last,
		[name] (const ggg::account& rhs) {
			return rhs.login().compare(name) == 0;
		}
	);
	int err = 0;
	if (it == last) {
		ret = NSS_STATUS_NOTFOUND;
		err = ENOENT;
	} else if (buflen < it->buffer_size()) {
		ret = NSS_STATUS_TRYAGAIN;
		err = ERANGE;
	} else {
		it->copy_to(result, buffer);
		ret = NSS_STATUS_SUCCESS;
	}
	*errnop = err;
	return ret;
}
