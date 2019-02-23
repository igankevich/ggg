#include <shadow.h>
#include <stddef.h>

#include <ggg/config.hh>
#include <ggg/nss/hierarchy_instance.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

	template <>
	struct entity_traits<account> {

		typedef account entity_type;
		typedef Database::row_stream_t stream_type;

		static inline stream_type
		all(Database* db) {
			return db->accounts();
		}

	};

}

namespace {

	typedef sqlite::rstream_iterator<ggg::account> iterator;
	typedef struct ::spwd entity_type;
	ggg::NSS_database<ggg::account> database;

}

NSS_ENUMERATE(sp, entity_type, Accounts)

NSS_GETENTBY_R(sp, nam)(
	const char* name,
	entity_type* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	int err;
	try {
		ggg::Database db(ggg::Database::File::Accounts);
		auto rstr = db.find_account(name);
		iterator first(rstr), last;
		if (first == last) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
		} else if (buflen < buffer_size(*first)) {
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
