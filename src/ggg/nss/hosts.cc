#include <netdb.h>

#include <ggg/nss/buffer.hh>
#include <ggg/nss/database.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

	template <>
	struct entity_traits<host_address> {

		typedef host_address entity_type;
		typedef Database::statement_type stream_type;
		typedef sqlite::row_iterator<host_address> iterator;

		static inline stream_type
		all(Database* db) {
			return db->host_addresses();
		}

	};

	inline size_t
	buffer_size(const ip_address& a) noexcept {
		return sizeof(sys::family_type) + a.size();
	}

	inline size_t
	buffer_size(const host_address& h) noexcept {
		return h.name().size() + 1 + buffer_size(h.address());
	}

	void
	copy_to(const host_address& a, struct ::hostent* lhs, char* buffer) {
		Pointer aliases[0];
		Pointer addresses[2];
		addresses[0].ptr = a.address().data();
		addresses[1].ptr = nullptr;
		Buffer buf(buffer);
		lhs->h_name = buf.write(a.name());
		lhs->h_aliases = buf.write(aliases, 0);
		lhs->h_addrtype = static_cast<int>(a.address().family());
		lhs->h_length = static_cast<int>(a.address().size());
		lhs->h_addr_list = buf.write(addresses, 2);
	}

}

namespace {

	typedef struct ::hostent entity_type;
	ggg::NSS_database<ggg::host_address> database;

}

NSS_SETENT(host)(int) { return database.open(ggg::Database::File::Entities); }
NSS_ENDENT(host)(void) { return database.close(); }

NSS_GETENT_R(host)(
	entity_type* result,
	char* buffer,
	size_t buflen,
	int* errnop,
	int* h_errnop
) {
	nss_status ret = NSS_STATUS_NOTFOUND;
	int err, h_err;
	auto& stream = database.stream();
	auto& entity = database.entity();
	try {
		if (database.empty()) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
			h_err = HOST_NOT_FOUND;
		} else if (buflen < buffer_size(entity)) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
			h_err = 0;
		} else {
			copy_to(entity, result, buffer);
			stream >> entity;
			ret = NSS_STATUS_SUCCESS;
			err = 0;
			h_err = 0;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		err = ENOENT;
		h_err = NO_RECOVERY;
	}
	*errnop = err;
	*h_errnop = h_err;
	return ret;
}


NSS_FUNCTION(gethostbyname2_r)(
	const char* name,
	int af,
	entity_type* result,
	char* buffer,
	size_t buflen,
	int* errnop,
	int* h_errnop
) {
	nss_status ret;
	int err, h_err;
	try {
		sys::family_type family = static_cast<sys::family_type>(af);
		ggg::Database db(ggg::Database::File::Entities);
		auto rstr = db.find_ip_address(name, family);
		auto first = rstr.begin<ggg::ip_address>();
		if (first == rstr.end<ggg::ip_address>()) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
			h_err = HOST_NOT_FOUND;
		} else {
			ggg::host_address ha(*first, name);
			if (buflen < buffer_size(ha)) {
				ret = NSS_STATUS_TRYAGAIN;
				err = ERANGE;
				h_err = 0;
			} else {
				copy_to(ha, result, buffer);
				ret = NSS_STATUS_SUCCESS;
				err = 0;
			}
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		err = ENOENT;
		h_err = NO_RECOVERY;
	}
	*h_errnop = h_err;
	*errnop = err;
	return ret;
}

NSS_FUNCTION(gethostbyname_r)(
	const char* name,
	entity_type* result,
	char* buffer,
	size_t buflen,
	int* errnop,
	int* h_errnop
) {
	return NSS_NAME(gethostbyname2_r)(
		name,
		AF_INET,
		result,
		buffer,
		buflen,
		errnop,
		h_errnop
	);
}

NSS_FUNCTION(gethostbyaddr_r)(
	const void* addr,
	socklen_t len,
	int af,
	entity_type* result,
	char* buffer,
	size_t buflen,
	int* errnop,
	int* h_errnop
) {
	nss_status ret;
	int err, h_err;
	try {
		sys::family_type family = static_cast<sys::family_type>(af);
		ggg::Database db(ggg::Database::File::Entities);
		ggg::ip_address address{family};
		if (address.size() != len) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
			h_err = HOST_NOT_FOUND;
		} else {
			std::string name;
			std::memcpy(address.data(), addr, len);
			auto rstr = db.find_host_name(address);
			if (rstr.step() == sqlite::errc::done) {
				ret = NSS_STATUS_NOTFOUND;
				err = ENOENT;
				h_err = HOST_NOT_FOUND;
			} else {
				sqlite::cstream cstr(rstr);
				cstr >> name;
				if (buflen < buffer_size(address)) {
					ret = NSS_STATUS_TRYAGAIN;
					err = ERANGE;
					h_err = 0;
				} else {
					ggg::host_address ha(address, name);
					copy_to(ha, result, buffer);
					ret = NSS_STATUS_SUCCESS;
					err = 0;
				}
			}
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		err = ENOENT;
		h_err = NO_RECOVERY;
	}
	*h_errnop = h_err;
	*errnop = err;
	return ret;
}
