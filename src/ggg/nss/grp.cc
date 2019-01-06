#include <cstdlib>
#include <limits>

#include <ggg/config.hh>
#include <ggg/nss/grp.hh>
#include <ggg/nss/hierarchy_instance.hh>

#if defined(GGG_DEBUG_INITGROUPS)
#include <iostream>
#endif

using ggg::database;

namespace {

	ggg::Database::row_stream_t rstr;
	ggg::Database::group_container_t all_groups;
	ggg::Database::group_container_t::iterator first, last;

	inline void
	init() {
		if (!database.is_open()) {
			database.open(ggg::Database::File::Entities);
			all_groups = database.groups();
			first = all_groups.begin();
			last = all_groups.end();
		}
	}

}

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, gr) {
	nss_status ret;
	try {
		init();
		ret = NSS_STATUS_SUCCESS;
	} catch (...) {
		ret = NSS_STATUS_UNAVAIL;
	}
	return ret;
}

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, gr) {
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

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, gr)(
	struct ::group* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	nss_status ret = NSS_STATUS_NOTFOUND;
	int err;
	try {
		if (first == last) {
			ret = NSS_STATUS_NOTFOUND;
			err = ENOENT;
		} else if (buflen < ggg::buffer_size(first->second)) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
		} else {
			ggg::copy_to(first->second, result, buffer);
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

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, gr, gid)(
	::gid_t gid,
	struct ::group* result,
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
		} else if (buflen < ggg::buffer_size(gr)) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
		} else {
			ggg::copy_to(gr, result, buffer);
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

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, gr, nam)(
	const char* name,
	struct ::group* result,
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
		} else if (buflen < ggg::buffer_size(gr)) {
			ret = NSS_STATUS_TRYAGAIN;
			err = ERANGE;
		} else {
			ggg::copy_to(gr, result, buffer);
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

NSS_MODULE_FUNCTION_INITGROUPS(MODULE_NAME)(
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
