#include <ggg/core/database.hh>
#include <ggg/core/host.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/entity_traits.hh>
#include <ggg/nss/etherent.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

	template <>
	size_t
	buffer_size<host>(const host& h) noexcept {
		return h.name().size() + 1 + h.address().size();
	}

	template <>
	void
	copy_to<host>(const host& ent, struct ::etherent* lhs, char* buffer) {
		Buffer buf(buffer);
		lhs->e_name = buf.write(ent.name());
		std::memcpy(
			lhs->e_addr.ether_addr_octet,
			ent.address().data(),
			ent.address().size()
		);
	}

}

NSS_FUNCTION(gethostton_r)(
	const char* name,
	struct ::etherent* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	int err;
	try {
		ggg::Database db(ggg::Database::File::Entities);
		auto rstr = db.find_host(name);
		ggg::host_iterator first(rstr), last;
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

NSS_FUNCTION(getntohost_r)(
	const struct ::ether_addr* address,
	struct ::etherent* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	int err;
	try {
		ggg::Database db(ggg::Database::File::Entities);
		auto rstr = db.find_host(
			sys::ethernet_address(address->ether_addr_octet)
		);
		ggg::host_iterator first(rstr), last;
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
