#include <shadow.h>
#include <stddef.h>

#include <ggg/config.hh>
#include <ggg/core/days.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

	template <>
	struct entity_traits<account> {

		typedef account entity_type;
		typedef Database::statement_type stream_type;
		typedef sqlite::row_iterator<account> iterator;

		static inline stream_type
		all(Database* db) {
			return db->accounts();
		}

	};

}

namespace {

	typedef sqlite::row_iterator<ggg::account> iterator;
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
			copy_to(*first, result, buffer);
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
