#include <shadow.h>
#include <stddef.h>

#include <ggg/bits/bufcopy.hh>
#include <ggg/config.hh>
#include <ggg/core/days.hh>
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

	template <>
	size_t
	buffer_size<account>(const account& a) noexcept {
		return a.login().size() + 1 + a.password().size() + 1;
	}

	template <>
	void
	copy_to<account>(const account& ent, struct ::spwd* lhs, char* buffer) {
		using bits::Buffer;
		Buffer buf(buffer);
		lhs->sp_namp = buf.write(ent.login());
		lhs->sp_pwdp = buf.write("");
		set_days(lhs->sp_lstchg, ent.last_change());
		set_days(lhs->sp_min, ent.min_change());
		set_days(lhs->sp_max, ent.max_change());
		set_days(lhs->sp_warn, ent.warn_change());
		set_days(lhs->sp_inact, ent.max_inactive());
		set_days(lhs->sp_expire, ent.expire());
	}

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
