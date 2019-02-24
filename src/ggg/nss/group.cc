#include <cstdlib>
#include <grp.h>
#include <limits>
#include <stddef.h>

#include <ggg/bits/bufcopy.hh>
#include <ggg/config.hh>
#include <ggg/nss/hierarchy_instance.hh>
#include <ggg/nss/nss.hh>

#if defined(GGG_DEBUG_INITGROUPS)
#include <iostream>
#endif

namespace ggg {

	class Group_stream {

	private:
		typedef ggg::Database::group_container_t container_type;
		typedef container_type::iterator iterator;

	private:
		container_type _groups;
		iterator _first, _last;
		bool _good = true;

	public:

		Group_stream() = default;

		inline
		Group_stream(container_type&& groups):
		_groups(std::move(groups)),
		_first(_groups.begin()),
		_last(_groups.end()),
		_good(_first != _last) {}

		inline
		Group_stream(Group_stream&& rhs):
		_groups(std::move(rhs._groups)),
		_first(_groups.begin()),
		_last(_groups.end()),
		_good(rhs._good) {}

		inline Group_stream&
		operator=(Group_stream&& rhs) {
			this->_groups = std::move(rhs._groups);
			this->_first = this->_groups.begin();
			this->_last = this->_groups.end();
			this->_good = rhs._good;
			return *this;
		}

		inline Group_stream&
		operator>>(group& rhs) {
			if (this->_first == this->_last) {
				this->_good = false;
				return *this;
			}
			rhs = std::move(this->_first->second);
			++this->_first;
			return *this;
		}

		inline bool
		good() const {
			return this->_good;
		}

		inline void
		close() {
			this->_groups.clear();
			this->_first = this->_groups.begin();
			this->_last = this->_groups.end();
		}

	};

	template <>
	struct entity_traits<group> {

		typedef group entity_type;
		typedef Group_stream stream_type;

		static inline stream_type
		all(Database* db) {
			return Group_stream(db->groups());
		}

	};

	template <>
	size_t
	buffer_size<group>(const group& gr) noexcept {
		using bits::pointer;
		size_t sum = 0;
		sum += gr.name().size() + 1;
		sum += gr.password().size() + 1;
		for (const std::string& member : gr.members()) {
			sum += member.size() + 1;
		}
		sum += sizeof(pointer)*(gr.members().size() + 1);
		sum += alignof(pointer) - 1;
		return sum;
	}

	template <>
	void
	copy_to<group>(const group& gr, struct ::group* lhs, char* buffer) {
		using bits::pointer;
		using bits::Buffer;
		using bits::Vector;
		Buffer buf(buffer);
		lhs->gr_name = buf.write(gr.name());
		lhs->gr_passwd = buf.write("");
		auto n = gr.members().size();
		std::unique_ptr<Vector[]> mem(new Vector[n]);
		size_t i = 0;
		for (const auto& member : gr.members()) {
			mem[i].ptr = buf.write(member.data());
			++i;
		}
		lhs->gr_mem = buf.write(mem.get(), n);
		lhs->gr_gid = gr.id();
	}

}

namespace {

	typedef struct ::group entity_type;
	ggg::NSS_database<ggg::group> database;

}

NSS_ENUMERATE(gr, entity_type, Entities)

NSS_GETENTBY_R(gr, gid)(
	::gid_t gid,
	entity_type* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret;
	int err;
	try {
		ggg::Database db(ggg::Database::File::Entities);
		ggg::group gr;
		if (!db.find_group(gid, gr)) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
		} else if (buflen < buffer_size(gr)) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
		} else {
			copy_to(gr, result, buffer);
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

NSS_GETENTBY_R(gr, nam)(
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
		ggg::group gr;
		if (!db.find_group(name, gr)) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
		} else if (buflen < buffer_size(gr)) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
		} else {
			copy_to(gr, result, buffer);
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

NSS_FUNCTION(initgroups_dyn)(
	const char* user,
	gid_t group,
	long int* start_ptr,
	long int* size_ptr,
	gid_t** groupsp,
	long int max_size,
	int* errnop
) {
	const long int default_size = 4096 / sizeof(::gid_t);
	nss_status ret = NSS_STATUS_SUCCESS;
	int err = 0;
	if (max_size <= 0) {
		max_size = std::numeric_limits<long int>::max();
	}
	auto& start = *start_ptr;
	auto& size = *size_ptr;
	try {
		#if defined(GGG_DEBUG_INITGROUPS)
		std::clog << "user=" << user << std::endl;
		std::clog << "group=" << group << std::endl;
		std::clog << "start=" << start << std::endl;
		std::clog << "size=" << size << std::endl;
		std::clog << "max_size=" << max_size << std::endl;
		#endif
		ggg::Database db(ggg::Database::File::Entities);
		auto rstr = db.find_parent_entities(user);
		ggg::user_iterator first(rstr), last;
		while (first != last) {
			if (first->id() == group) {
				++first;
				continue;
			}
			if (start >= max_size) {
				ret = NSS_STATUS_TRYAGAIN;
				err = ERANGE;
				break;
			}
			if (start >= size) {
				auto newsize = (size == 0) ? default_size : (size*2);
				auto newgroups = std::realloc(*groupsp, newsize*sizeof(gid_t));
				if (!newgroups) {
					ret = NSS_STATUS_TRYAGAIN;
					err = ERANGE;
					break;
				}
				size = newsize;
				*groupsp = reinterpret_cast<gid_t*>(newgroups);
			}
			(*groupsp)[start++] = first->id();
			++first;
		}
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
		err = ENOENT;
	}
	*errnop = err;
	return ret;
}
