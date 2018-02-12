#include "hierarchy.hh"

#include <pwd.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <stdexcept>

#include <unistdx/fs/idirtree>
#include <unistdx/it/intersperse_iterator>

#include <ggg/bits/io.hh>
#include <ggg/bits/to_bytes.hh>
#include <ggg/core/sets.hh>

namespace {

	template <class Ch>
	struct log_traits;

	template <>
	struct log_traits<char> {
		inline static std::ostream&
		log() noexcept {
			return std::clog;
		}
	};

	template <>
	struct log_traits<wchar_t> {
		inline static std::wostream&
		log() noexcept {
			return std::wclog;
		}
	};

	template <class Ch>
	struct delimiter_traits;

	template <>
	struct delimiter_traits<char> {

		inline static const char*
		newline() noexcept {
			return "\n";
		}

	};

	template <>
	struct delimiter_traits<wchar_t> {

		inline static const wchar_t*
		newline() noexcept {
			return L"\n";
		}

	};

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
void
ggg::basic_hierarchy<Ch>
::read() {
	bits::wcvt_type cv;
	sys::idirtree tree;
	try {
		tree.open(sys::path(bits::to_bytes<char>(cv, this->_root)));
	} catch (...) {
		log_traits<Ch>::log() << "Skipping bad hierarchy root " << _root << std::endl;
		return;
	}
	while (!tree.eof()) {
		tree.clear();
		sys::pathentry entry;
		bool success = false;
		while (!success) {
			if (tree >> entry) {
				try {
					this->process_entry(entry, cv, success);
				} catch (...) {
					std::clog << "Skipping bad file " << entry << std::endl;
				}
			} else {
				if (tree.eof()) {
					success = true;
				}
			}
		}
	}
	this->process_links();
	this->filter_entities();
	this->generate_groups();
	this->process_nested_groups();
	this->filter_groups();
	this->_isopen = true;
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
							log_traits<Ch>::log() << "Nest "
								<< group.name() << " and " << member << std::endl;
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
	const sys::pathentry& entry,
	entity::wcvt_type& cv,
	bool& success
) {
	canonical_path_type filepath(entry.getpath());
	if (filepath.is_relative_to(_root)) {
		switch (sys::get_file_type(entry)) {
		case sys::file_type::regular: {
			std::basic_ifstream<Ch> in(
				bits::to_bytes<char>(cv, filepath),
				std::ios_base::in | std::ios_base::binary
			);
			if (in.is_open()) {
				entity_type ent;
				while (in >> ent) {
					ent.origin(filepath);
					add_entity(ent, filepath, cv);
				}
				entity byte_ent(entry.name());
				add_entity(entity_type(byte_ent, cv), filepath.dirname(), cv);
				success = true;
			}
			break;
		}
		case sys::file_type::symbolic_link: {
			entity byte_ent(entry.name());
			byte_ent.origin(entry.getpath());
			entity_type ent(byte_ent, cv);
			add_link(ent, canonical_path_type(entry.dirname()), cv);
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
	const canonical_path_type& filepath,
	entity::wcvt_type& cv
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
		result->second.emplace(bits::to_bytes<Ch>(cv, group));
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
		log_traits<Ch>::log() << "unlinking " << ent.origin() << std::endl;
	}
	int ret = ::unlink(ent.origin());
	if (ret == -1) {
		throw std::system_error(errno, std::system_category());
	}
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::erase_regular(const entity_type& ent) {
	typedef std::istream_iterator<entity_type,Ch> iterator;
	typedef std::ostream_iterator<entity_type,Ch> oiterator;
	std::string new_origin;
	canonical_path_type origin(ent.origin());
	new_origin.append(origin);
	new_origin.append(".new");
	bits::wcvt_type cv;
	bits::check(origin, R_OK | W_OK);
	bits::check(new_origin, R_OK | W_OK, false);
	std::basic_ifstream<Ch> file;
	std::basic_ofstream<Ch> file_new;
	try {
		file.exceptions(std::ios::badbit);
		file.imbue(std::locale::classic());
		file.open(origin, std::ios_base::in | std::ios_base::binary);
		file_new.exceptions(std::ios::badbit);
		file_new.imbue(std::locale::classic());
		file_new.open(new_origin, std::ios_base::out | std::ios_base::binary);
		std::copy_if(
			iterator(file),
			iterator(),
			oiterator(file_new, delimiter_traits<Ch>::newline()),
			[&ent,this] (const entity_type& rhs) {
			    bool match = rhs.name() == ent.name();
			    if (match && this->_verbose) {
			        log_traits<Ch>::log()
			        << "removing " << rhs.name() << ':' << rhs.id()
			        << " from " << ent.origin()
			        << std::endl;
				}
			    return !match;
			}
		);
	} catch (...) {
		bits::throw_io_error(file, "unable to remove entity");
	}
	ggg::bits::rename(new_origin, origin);
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
	typename entity_type::wcvt_type cv;
	entity byte_ent(ent, cv);
	if (struct ::passwd* pw = ::getpwnam(byte_ent.name().data())) {
		if (pw->pw_uid < this->_minuid || pw->pw_gid < this->_mingid) {
			throw std::invalid_argument("conflicting system user");
		}
	}
	this->update_regular(ent, result->origin());
}

template <class Ch>
void
ggg::basic_hierarchy<Ch>
::update_regular(
	const entity_type& ent,
	path_type ent_origin
) {
	typedef std::istream_iterator<entity_type,Ch> iterator;
	typedef std::ostream_iterator<entity_type,Ch> oiterator;
	std::string new_origin;
	canonical_path_type origin(ent_origin);
	new_origin.append(origin);
	new_origin.append(".new");
	bits::check(origin, R_OK | W_OK);
	bits::check(new_origin.data(), R_OK | W_OK, false);
	std::basic_ifstream<Ch> file;
	std::basic_ofstream<Ch> file_new;
	try {
		file.exceptions(std::ios::badbit);
		file.imbue(std::locale::classic());
		file.open(origin, std::ios_base::in | std::ios_base::binary);
		file_new.exceptions(std::ios::badbit);
		file_new.imbue(std::locale::classic());
		file_new.open(new_origin, std::ios_base::out | std::ios_base::binary);
		std::transform(
			iterator(file),
			iterator(),
			oiterator(file_new, delimiter_traits<Ch>::newline()),
			[&ent,&ent_origin,this] (const entity_type& rhs) {
			    bool match = rhs.name() == ent.name();
			    if (match && this->_verbose) {
			        log_traits<Ch>::log()
			        << "updating " << rhs.name() << ':' << rhs.id()
			        << " in " << ent_origin
			        << std::endl;
				}
			    return match ? ent : rhs;
			}
		);
	} catch (...) {
		bits::throw_io_error(file, "unable to update entity");
	}
	ggg::bits::rename(new_origin, origin);
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
::add(
	const entity_type& ent,
	const std::string& filename
) {
	if (filename.empty()) {
		throw std::invalid_argument("empty filename");
	}
	this->validate_entity(ent);
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
	typename entity_type::wcvt_type cv;
	entity byte_ent(ent, cv);
	if (::getpwnam(byte_ent.name().data())) {
		throw std::invalid_argument("conflicting system user");
	}
	this->append_entity(ent, filename);
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
		log_traits<Ch>::log()
		    << "appending " << ent.name() << ':' << ent.id()
		    << " to " << dest
		    << std::endl;
	}
	bits::append(ent, dest, "unable to add entity");
}

template class ggg::basic_hierarchy<char>;
template class ggg::basic_hierarchy<wchar_t>;

template std::basic_ostream<char>&
ggg::operator<<(
	std::basic_ostream<char>& out,
	const basic_hierarchy<char>& rhs
);

template std::basic_ostream<char>&
ggg::operator<<(std::basic_ostream<char>& out, const entity_pair<char>& rhs);
