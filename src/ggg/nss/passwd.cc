#include <pwd.h>
#include <stddef.h>

#include <ggg/config.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

	template <>
	struct entity_traits<entity> {

		typedef entity entity_type;
		typedef Database::row_stream_t stream_type;
		typedef sqlite::row_iterator<entity> iterator;

		static inline stream_type
		all(Database* db) {
			return db->entities();
		}

	};

	template <>
	size_t
	buffer_size<entity>(const entity& rhs) noexcept {
		return rhs.name().size() + 1
			   + rhs.password().size() + 1
			   + rhs.real_name().size() + 1
			   + rhs.home().size() + 1
			   + rhs.shell().size() + 1;
	}

	template <>
	void
	copy_to<entity,::passwd>(const entity& ent, passwd* lhs, char* buffer) {
		Buffer buf(buffer);
		lhs->pw_name = buf.write(ent.name());
		lhs->pw_passwd = buf.write(ent.password());
		#ifdef __linux__
		lhs->pw_gecos = buf.write(ent.real_name());
		#endif
		lhs->pw_dir = buf.write(ent.home());
		lhs->pw_shell = buf.write(ent.shell());
		lhs->pw_uid = ent.id();
		lhs->pw_gid = ent.gid();
	}

}

namespace {

	typedef struct ::passwd entity_type;
	ggg::NSS_database<ggg::entity> database;

}

NSS_ENUMERATE(pw, entity_type, Entities)

NSS_GETENTBY_R(pw, uid)(
	::uid_t uid,
	entity_type* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	int err;
	try {
		ggg::Database db(ggg::Database::File::Entities);
		auto rstr = db.find_user(uid);
		ggg::user_iterator first(rstr), last;
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

NSS_GETENTBY_R(pw, nam)(
	const char* name,
	entity_type* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	int err;
	try {
		ggg::Database db(ggg::Database::File::Entities);
		auto rstr = db.find_user(name);
		ggg::user_iterator first(rstr), last;
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
