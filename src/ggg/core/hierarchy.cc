#include "hierarchy.hh"

#include <pwd.h>

#include <iostream>
#include <iterator>
#include <locale>
#include <stdexcept>
#include <type_traits>

#include <unistdx/fs/idirtree>
#include <unistdx/it/intersperse_iterator>
#include <unistdx/io/fdstream>

#include <ggg/bits/io.hh>
#include <ggg/bits/to_bytes.hh>
#include <ggg/config.hh>
#include <ggg/core/sets.hh>
#include <ggg/core/timed_fildes.hh>

namespace {

	template <class Ch>
	inline sys::path
	cache_file_path(sys::canonical_path root, char suffix) {
		for (char& ch : root) {
			if (ch == '/') {
				ch = '_';
			}
		}
		root += '.';
		root += suffix;
		return sys::path(GGG_CACHE_DIRECTORY, "ggg", root);
	}

	template <class Ch>
	inline sys::path
	cache_file_path(sys::canonical_path root) {
		return std::is_same<char,Ch>::value
		       ? cache_file_path<Ch>(root, 'c')
			   : cache_file_path<Ch>(root, 'w');
	}

}


namespace ggg {

	template <class Container, class Ch>
	sys::basic_bstream<Ch>&
	operator<<(sys::basic_bstream<Ch>& out, const Container& rhs) {
		out << uint32_t(rhs.size());
		for (const auto& elem : rhs) {
			out << elem;
		}
		return out;
	}

	template <class Ch, class T>
	sys::basic_bstream<Ch>&
	operator>>(sys::basic_bstream<Ch>& in, std::set<T>& rhs) {
		rhs.clear();
		uint32_t n = 0;
		in >> n;
		for (uint32_t i=0; i<n; ++i) {
			T elem;
			in >> elem;
			rhs.emplace(elem);
		}
		return in;
	}

	template <class Ch, class K, class V>
	sys::basic_bstream<Ch>&
	operator>>(sys::basic_bstream<Ch>& in, std::map<K,V>& rhs) {
		rhs.clear();
		uint32_t n = 0;
		in >> n;
		for (uint32_t i=0; i<n; ++i) {
			K key;
			V val;
			in >> key >> val;
			rhs.emplace(key, val);
		}
		return in;
	}

}

template <class Ch>
std::basic_ostream<Ch>&
ggg::operator<<(std::basic_ostream<Ch>& out, const entity_pair<Ch>& rhs) {
	out << rhs.first << ':';
	std::copy(
		rhs.second.begin(),
		rhs.second.end(),
		sys::intersperse_iterator<std::basic_string<Ch>,Ch,Ch>(out, Ch(','))
	);
	return out;
}

template <class Ch>
ggg::basic_hierarchy<Ch>::~basic_hierarchy() {
	this->write_cache();
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::open(const char* root) {
	#ifndef NDEBUG
	assert(sys::path(root) != sys::path(GGG_ROOT));
	#endif
	this->_root = canonical_path_type(root);
	this->read_from_local_cache();
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::read_from_local_cache() {
	try {
		this->open_cache();
	} catch (const std::exception& err) {
		this->_cachestate = cache_state::error;
	}
	if (this->_cachestate == cache_state::expired &&
		this->_fsstate == fs_state::timed_out)
	{
		if (this->_verbose) {
			std::clog << "using expired cache" << std::endl;
		}
	} else if (this->_cachestate == cache_state::ok) {
		if (this->_verbose) {
			std::clog << "using cache" << std::endl;
		}
	} else if (this->_cachestate == cache_state::error ||
			   this->_cachestate == cache_state::expired)
	{
		if (this->_verbose) {
			std::clog << "update cache from file system" << std::endl;
		}
		this->clear();
		// read with infinite timeout
		this->_timeout = -std::chrono::milliseconds(1);
		this->read_from_file_system();
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::remove_cache() {
	std::remove(cache_file_path<char>(this->_root));
	this->_donotwritecache = true;
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::open_cache() {
	using std::chrono::seconds;
	sys::path cache = cache_file_path<Ch>(this->_root);
	sys::file_status st(cache);
	if (!st.is_regular()) {
		throw std::runtime_error("cache is not a regular file");
	}
	const auto dt = clock_type::now() - time_point(seconds(st.st_mtime));
	if (dt >= duration::zero() && dt <= this->_ttl) {
		this->read_cache(cache);
		this->_cachestate = cache_state::ok;
	} else {
		this->_cachestate = cache_state::expired;
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::read_cache(sys::path cache) {
	std::basic_filebuf<Ch> buf;
	buf.open(cache, std::ios_base::in | std::ios_base::binary);
	if (!buf.is_open()) {
		throw std::runtime_error("bad cache file");
	}
	sys::basic_bstream<Ch> in(&buf);
	in >> *this;
	this->_isopen = true;
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::write_cache() {
	if (this->is_open() &&
		!this->_donotwritecache &&
	    sys::this_process::user() == 0) {
		sys::path cache = cache_file_path<Ch>(this->_root);
		try {
			this->write_cache(cache);
		} catch (const std::exception& err) {
			// ignore errors
		}
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::write_cache(sys::path cache) {
	std::basic_filebuf<Ch> buf;
	buf.open(
		cache,
		std::ios_base::out | std::ios_base::binary |
		std::ios_base::trunc
	);
	if (!buf.is_open()) {
		return;
	}
	set_perms(cache, 0, 0, 0644);
	sys::basic_bstream<Ch> out(&buf);
	out << *this;
	out.flush();
	buf.close();
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::read_from_file_system() {
	sys::idirtree tree;
	tree.open(sys::path(this->_root));
	while (!tree.eof()) {
		tree.clear();
		sys::directory_entry entry;
		bool success = false;
		while (!success) {
			try {
				if (tree >> entry) {
					this->process_entry(tree.current_dir(), entry, success);
				} else {
					if (tree.eof()) {
						success = true;
					}
				}
			} catch (const core::timed_out& err) {
				this->_fsstate = fs_state::timed_out;
				this->clear();
				this->read_from_local_cache();
			} catch (...) {
				#ifndef NDEBUG
				std::clog
				    << "Skipping bad file "
				    << sys::path(tree.current_dir(), entry.name())
				    << std::endl;
				#endif
			}
		}
	}
	this->process_links();
	this->filter_entities();
	this->generate_groups();
	this->process_nested_groups();
	this->filter_groups();
	this->_isopen = true;
	this->_fsstate = fs_state::ok;
	this->write_cache();
}

template <class Ch>
std::basic_ostream<Ch>&
ggg::operator<<(std::basic_ostream<Ch>& out, const basic_hierarchy<Ch>& rhs) {
	typedef typename basic_hierarchy<Ch>::entity_pair_type entity_pair_type;
	typedef typename basic_hierarchy<Ch>::group_type group_type;
	out << "\nUsers:\n";
	std::copy(
		rhs._entities.begin(),
		rhs._entities.end(),
		std::ostream_iterator<entity_pair_type,Ch>(out, "\n")
	);
	out << "\nGroups:\n";
	std::copy(
		rhs._groups.begin(),
		rhs._groups.end(),
		std::ostream_iterator<group_type,Ch>(out, "\n")
	);
	return out;
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::process_links() {
	size_t num_insertions = 1;
	size_t old_size = 0;
	const size_t max_links = this->_maxlinks;
	for (size_t i=0; i<max_links && num_insertions > 0; ++i) {
		num_insertions = 0;
		for (const entity_pair_type& pair : this->_links) {
			const entity_type& link = pair.first;
			entity_set_type& s = _entities[link];
			old_size = s.size();
			set_union(s, pair.second);
			num_insertions += s.size() - old_size;
			for (entity_pair_type& pair2 : _entities) {
				entity_set_type& s2 = pair2.second;
				if (s2.count(link.name())) {
					old_size = s2.size();
					set_union(s2, pair.second);
					num_insertions += s2.size() - old_size;
				}
			}
		}
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::process_nested_groups() {
	size_t num_insertions = 1;
	const size_t max_links = this->_maxlinks;
	entity_set_type add, sub;
	for (size_t i=0; i<max_links && num_insertions > 0; ++i) {
		num_insertions = 0;
		for (const group_type& group : this->_groups) {
			auto& members = group.members();
			for (const auto& member : members) {
				if (member != group.name()) {
					auto result = this->_groups.find(group_type(member));
					if (result != this->_groups.end()) {
						const auto& s = result->members();
						if (!s.empty()) {
							#ifndef NDEBUG
							std::clog << "Nest " << group.name()
								<< " and " << member << std::endl;
							#endif
							set_union(add, s);
							sub.insert(member);
						}
					}
				}
			}
			if (!add.empty()) {
				const size_t old_size = members.size();
				set_difference(members, sub);
				set_union(members, add);
				num_insertions += members.size() - old_size;
				add.clear();
				sub.clear();
			}
		}
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::filter_entities() {
	erase_if(
		this->_entities,
		[this] (const entity_pair_type& rhs) {
		    return !rhs.first.has_id() || rhs.first.id() < this->_minuid;
		}
	);
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::generate_groups() {
	// add groups
	for (const entity_pair_type& pair : _entities) {
		const entity_type& ent = pair.first;
		_groups.emplace(ent.name(), ent.password(), ent.gid());
	}
	// add group members
	for (const entity_pair_type& pair : _entities) {
		const entity_type& member = pair.first;
		for (const string_type& grname : pair.second) {
			auto result = _groups.find(group_type(grname));
			if (result != _groups.end()) {
				result->push(member.name());
			}
		}
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::filter_groups() {
	// remove non-existent groups
	for (entity_pair_type& pair : this->_entities) {
		erase_if(
			pair.second,
			[this] (const string_type& grname) {
			    return this->_groups.find(group_type(grname)) ==
			    this->_groups.end();
			}
		);
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::process_entry(
	const sys::path& dir,
	const sys::directory_entry& entry,
	bool& success
) {
	using core::timed_fildes;
	using core::timed_ifdstream;
	sys::path filepath0(dir, entry.name());
	canonical_path_type filepath(filepath0);
	if (filepath.is_relative_to(this->_root)) {
		switch (sys::get_file_type(dir, entry)) {
		case sys::file_type::regular: {
			timed_fildes fd(filepath, sys::open_flag::read_only);
			fd.timeout(this->_timeout);
			timed_ifdstream in(std::move(fd));
			entity_type ent;
			while (in >> ent) {
				ent.origin(filepath);
				add_entity(ent, filepath);
			}
			entity byte_ent(entry.name());
			add_entity(entity_type(byte_ent), filepath.dirname());
			success = true;
			break;
		}
		case sys::file_type::symbolic_link: {
			entity ent(entry.name());
			ent.origin(filepath0);
			add_link(ent, filepath.dirname());
			break;
		}
		default: break;
		}
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::add(
	map_type& container,
	const entity_type& ent,
	const canonical_path_type& filepath
) {
	std::string dirs(
		filepath,
		std::min(filepath.size(), this->_root.size() + 1)
	);
	std::stringstream tmp;
	tmp.imbue(std::locale::classic());
	tmp << dirs;
	auto result = container.find(ent);
	if (result != container.end()) {
		const_cast<entity_type&>(result->first).merge(ent);
	} else {
		result = container.emplace(ent, entity_set_type()).first;
	}
	std::string group;
	while (std::getline(tmp, group, sys::file_separator)) {
		result->second.emplace(group);
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::erase(const char_type* name) {
	const_iterator result = this->find_by_name(name);
	if (result != this->end() && result->has_origin()) {
		this->erase_regular(*result);
	}
	result = this->find_link_by_name(name);
	if (result != this->_links.end() && result->has_origin()) {
		this->erase_link(*result);
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::erase_link(const entity_type& ent) {
	if (this->_verbose) {
		std::clog << "unlinking " << ent.origin() << std::endl;
	}
	int ret = ::unlink(ent.origin());
	if (ret == -1) {
		throw std::system_error(errno, std::system_category());
	}
	this->remove_cache();
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::erase_regular(const entity_type& ent) {
	bits::erase<entity_type,char_type>(ent, this->_verbose, 0644);
	this->remove_cache();
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::validate_entity(const entity_type& ent) {
	if (!(ent.id() >= this->_minuid)) {
		throw std::invalid_argument("bad uid");
	}
	if (!(ent.gid() >= this->_mingid)) {
		throw std::invalid_argument("bad gid");
	}
	if (!ent.has_valid_name()) {
		throw std::invalid_argument("bad name");
	}
	if (ent.home().empty() || ent.home().front() != sys::file_separator) {
		throw std::invalid_argument("bad home directory");
	}
	if (ent.shell().empty() || ent.shell().front() != sys::file_separator) {
		throw std::invalid_argument("bad shell");
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::update(const entity_type& ent) {
	this->validate_entity(ent);
	const_iterator result1 = this->find_by_name(ent.name().data());
	const_iterator result2 = this->find_by_uid(ent.id());
	const_iterator last = this->cend();
	const bool exists1 = result1 != last;
	const bool exists2 = result2 != last;
	if (!exists1 && !exists2) {
		throw std::invalid_argument("bad entity");
	}
	if (exists1 && exists2 && *result1 != *result2) {
		throw std::invalid_argument("conflicting name/uid pair");
	}
	const_iterator result = exists1 ? result1 : result2;
	if (!result->has_origin()) {
		throw std::invalid_argument("bad origin");
	}
	if (struct ::passwd* pw = ::getpwnam(ent.name().data())) {
		if (pw->pw_uid < this->_minuid || pw->pw_gid < this->_mingid) {
			throw std::invalid_argument("conflicting system user");
		}
	}
	this->update_regular(ent, result->origin());
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::update_regular(const entity_type& ent, path_type ent_origin) {
	entity_type tmp(ent);
	tmp.origin(ent_origin);
	bits::update<entity_type,char_type>(tmp, this->_verbose, 0644);
	this->remove_cache();
}

template <class Ch>
ggg::basic_entity<Ch>
ggg::basic_hierarchy<Ch>
::generate(const char_type* name) {
	sys::uid_type id = this->next_uid();
	entity_type ent(name, id, id);
	return ent;
}

template <class Ch>
sys::uid_type
ggg::basic_hierarchy<Ch>
::next_uid() const {
	sys::uid_type uid;
	const_iterator result = std::max_element(
		this->begin(),
		this->end(),
		[] (const entity_type& lhs, const entity_type& rhs) {
		    return lhs.id() < rhs.id();
		}
	                        );
	if (result == this->end()) {
		uid = this->_minuid;
	} else {
		uid = result->id();
	}
	if (uid < this->_minuid) {
		uid = this->_minuid;
	}
	if (uid == std::numeric_limits<sys::uid_type>::max()) {
		throw std::overflow_error("bad uid");
	}
	return uid + 1;
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::add(const entity_type& ent) {
	this->validate_entity(ent);
	if (ent.origin().empty()) {
		throw std::invalid_argument("empty filename");
	}
	const_iterator result1 = this->find_by_name(ent.name().data());
	const_iterator result2 = this->find_by_uid(ent.id());
	const_iterator last = this->cend();
	const bool exists1 = result1 != last;
	const bool exists2 = result2 != last;
	if (exists1 || exists2) {
		throw std::invalid_argument(
				  "entity with the same name/uid already exists"
		);
	}
	if (::getpwnam(ent.name().data())) {
		throw std::invalid_argument("conflicting system user");
	}
	this->append_entity(ent, ent.origin());
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::append_entity(
	const entity_type& ent,
	const std::string& filename
) {
	sys::path dest(this->_root, filename);
	if (this->verbose()) {
		std::clog
		    << "appending " << ent.name() << ':' << ent.id()
		    << " to " << dest
		    << std::endl;
	}
	bits::append(ent, dest, "unable to add entity", 0644);
	this->remove_cache();
}

template <class Ch>
sys::basic_bstream<Ch>&
ggg::operator<<(sys::basic_bstream<Ch>& out, const entity_pair<Ch>& rhs) {
	return out << rhs.first << rhs.second;
}

template <class Ch>
sys::basic_bstream<Ch>&
ggg::operator<<(sys::basic_bstream<Ch>& out, const basic_hierarchy<Ch>& rhs) {
	return out << rhs._entities << rhs._links << rhs._groups;
}

template <class Ch>
sys::basic_bstream<Ch>&
ggg::operator>>(sys::basic_bstream<Ch>& in, basic_hierarchy<Ch>& rhs) {
	return in >> rhs._entities >> rhs._links >> rhs._groups;
}

template class ggg::basic_hierarchy<char>;

template std::basic_ostream<char>&
ggg::operator<<(
	std::basic_ostream<char>& out,
	const basic_hierarchy<char>& rhs
);

template std::basic_ostream<char>&
ggg::operator<<(std::basic_ostream<char>& out, const entity_pair<char>& rhs);

template sys::basic_bstream<char>&
ggg::operator<<(sys::basic_bstream<char>& out, const entity_pair<char>& rhs);

template sys::basic_bstream<char>&
ggg::operator<<(
	sys::basic_bstream<char>& out,
	const
	basic_hierarchy<char>& rhs
);

template sys::basic_bstream<char>&
ggg::operator>>(sys::basic_bstream<char>& in, basic_hierarchy<char>& rhs);
