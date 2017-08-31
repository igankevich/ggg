#include "hierarchy.hh"

#include <unistdx/it/intersperse_iterator>
#include <unistdx/fs/idirtree>
#include <iostream>
#include <iterator>
#include <locale>
#include <stdexcept>
#include <pwd.h>

#include "bits/io.hh"

std::ostream&
ggg::operator<<(std::ostream& out, const entity_pair& rhs) {
	out << rhs.first << ':';
	std::copy(
		rhs.second.begin(),
		rhs.second.end(),
		sys::intersperse_iterator<std::string>(out, ",")
	);
	return out;
}

void
ggg::Hierarchy::read() {
	sys::idirtree tree;
	try {
		tree.open(_root);
	} catch (...) {
		std::clog << "Skipping bad hierarchy root " << _root << std::endl;
		return;
	}
	while (!tree.eof()) {
		tree.clear();
		sys::pathentry entry;
		bool success = false;
		while (!success) {
			if (tree >> entry) {
				try {
					this->process_entry(entry, success);
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
	process_links();
	filter_entities();
	generate_groups();
	filter_groups();
	_isopen = true;
}

std::ostream&
ggg::operator<<(std::ostream& out, const Hierarchy& rhs) {
	out << "\nUsers:\n";
	std::copy(
		rhs._entities.begin(),
		rhs._entities.end(),
		std::ostream_iterator<entity_pair>(out, "\n")
	);
	out << "\nGroups:\n";
	std::copy(
		rhs._groups.begin(),
		rhs._groups.end(),
		std::ostream_iterator<group>(out, "\n")
	);
	return out;
}

void
ggg::Hierarchy::process_links() {
	size_t num_insertions = 1;
	size_t old_size = 0;
	for (size_t i=0; i<_maxlinks && num_insertions > 0; ++i) {
		num_insertions = 0;
		for (const entity_pair& pair : _links) {
			const entity& link = pair.first;
			std::set<std::string>& s = _entities[link];
			old_size = s.size();
			s.insert(pair.second.begin(), pair.second.end());
			num_insertions += s.size() - old_size;
			for (entity_pair& pair2 : _entities) {
				std::set<std::string>& s2 = pair2.second;
				if (s2.count(link.name())) {
					old_size = s2.size();
					s2.insert(pair.second.begin(), pair.second.end());
					num_insertions += s2.size() - old_size;
				}
			}
		}
	}
}

void
ggg::Hierarchy::filter_entities() {
	erase_if(
		_entities,
		[this] (const entity_pair& rhs) {
			return !rhs.first.has_id() || rhs.first.id() < _minuid;
		}
	);
}

void
ggg::Hierarchy::generate_groups() {
	// add groups
	for (const entity_pair& pair : _entities) {
		const entity& ent = pair.first;
		_groups.emplace(ent.name(), ent.password(), ent.gid());
	}
	// add group members
	for (const entity_pair& pair : _entities) {
		const entity& member = pair.first;
		for (const std::string& grname : pair.second) {
			auto result = _groups.find(group(grname));
			if (result != _groups.end()) {
				result->push(member.name());
			}
		}
	}
}

void
ggg::Hierarchy::filter_groups() {
	/*
	// add entities to the groups with the same names
	for (const group& rhs : this->_groups) {
		const_iterator result = this->find_by_name(rhs.name().data());
		if (result != this->_entities.end()) {
			rhs.push(result->name());
		}
	}
	// remove empty groups
	erase_if(
		this->_groups,
		[] (const group& rhs) {
			return rhs.members().empty();
		}
	);
	*/
	/*
	// remove duplicate entities
	for (const group& grp : _groups) {
		_entities.erase(entity(grp.name().data()));
	}
	*/
	// remove non-existent groups
	for (entity_pair& pair : _entities) {
		erase_if(
			pair.second,
			[this] (const std::string& grname) {
				return _groups.find(group(grname)) == _groups.end();
			}
		);
	}
}

void
ggg::Hierarchy::process_entry(const sys::pathentry& entry, bool& success) {
	sys::canonical_path filepath(entry.getpath());
	if (filepath.is_relative_to(_root)) {
		switch (sys::get_file_type(entry)) {
			case sys::file_type::regular: {
				std::ifstream in(filepath);
				in.imbue(std::locale::classic());
				if (in.is_open()) {
					entity ent;
					while (in >> ent) {
						ent.origin(filepath);
						add_entity(ent, filepath);
					}
					add_entity(entity(entry.name()), filepath.dirname());
					success = true;
				}
				break;
			}
			case sys::file_type::symbolic_link: {
				entity ent(entry.name());
				ent.origin(entry.getpath());
				add_link(ent, sys::canonical_path(entry.dirname()));
				break;
			}
			default: break;
		}
	}
}

void
ggg::Hierarchy::add(
	map_type& container,
	const entity& ent,
	const sys::canonical_path& filepath
) {
	std::string dirs(
		filepath,
		std::min(filepath.size(), _root.size() + 1)
	);
	std::stringstream tmp;
	tmp.imbue(std::locale::classic());
	tmp << dirs;
	auto result = container.find(ent);
	if (result != container.end()) {
		const_cast<entity&>(result->first).merge(ent);
	} else {
		result = container.emplace(ent, std::set<std::string>()).first;
	}
	std::string group;
	while (std::getline(tmp, group, sys::file_separator)) {
		result->second.emplace(group);
	}
}

void
ggg::Hierarchy::erase(const char* name) {
	const_iterator result = this->find_by_name(name);
	if (result != this->end() && result->has_origin()) {
		this->erase_regular(*result);
	}
	result = this->find_link_by_name(name);
	if (result != this->_links.end() && result->has_origin()) {
		this->erase_link(*result);
	}
}

void
ggg::Hierarchy::erase_link(const entity& ent) {
	if (this->_verbose) {
		std::clog << "unlinking " << ent.origin() << std::endl;
	}
	int ret = ::unlink(ent.origin());
	if (ret == -1) {
		throw std::system_error(errno, std::system_category());
	}
}

void
ggg::Hierarchy::erase_regular(const entity& ent) {
	typedef std::istream_iterator<entity> iterator;
	typedef std::ostream_iterator<entity> oiterator;
	std::string new_origin;
	sys::canonical_path origin(ent.origin());
	new_origin.append(origin.dirname());
	new_origin.push_back('.');
	new_origin.append(origin.basename());
	new_origin.append(".new");
	bits::check(origin, R_OK | W_OK);
	bits::check(new_origin.data(), R_OK | W_OK, false);
	std::ifstream file;
	std::ofstream file_new;
	try {
		file.exceptions(std::ios::badbit);
		file.imbue(std::locale::classic());
		file.open(origin);
		file_new.exceptions(std::ios::badbit);
		file_new.imbue(std::locale::classic());
		file_new.open(new_origin);
		std::copy_if(
			iterator(file),
			iterator(),
			oiterator(file_new, "\n"),
			[&ent,this] (const entity& rhs) {
				bool match = rhs.name() == ent.name();
				if (match && this->_verbose) {
					std::clog
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
	ggg::bits::rename(new_origin.data(), origin);
}

void
ggg::Hierarchy::validate_entity(const entity& ent) {
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

void
ggg::Hierarchy::update(const entity& ent) {
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

void
ggg::Hierarchy::update_regular(const entity& ent, sys::path ent_origin) {
	typedef std::istream_iterator<entity> iterator;
	typedef std::ostream_iterator<entity> oiterator;
	std::string new_origin;
	sys::canonical_path origin(ent_origin);
	new_origin.append(origin.dirname());
	new_origin.push_back('.');
	new_origin.append(origin.basename());
	new_origin.append(".new");
	bits::check(origin, R_OK | W_OK);
	bits::check(new_origin.data(), R_OK | W_OK, false);
	std::ifstream file;
	std::ofstream file_new;
	try {
		file.exceptions(std::ios::badbit);
		file.imbue(std::locale::classic());
		file.open(origin);
		file_new.exceptions(std::ios::badbit);
		file_new.imbue(std::locale::classic());
		file_new.open(new_origin);
		std::transform(
			iterator(file),
			iterator(),
			oiterator(file_new, "\n"),
			[&ent,&ent_origin,this] (const entity& rhs) {
				bool match = rhs.name() == ent.name();
				if (match && this->_verbose) {
					std::clog
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
	ggg::bits::rename(new_origin.data(), origin);
}

ggg::entity
ggg::Hierarchy::generate(const char* name) {
	sys::uid_type id = this->next_uid();
	std::clog << "id=" << id << std::endl;
	entity ent(name, id, id);
	return ent;
}

sys::uid_type
ggg::Hierarchy::next_uid() const {
	sys::uid_type uid;
	const_iterator result = std::max_element(
		this->begin(),
		this->end(),
		[] (const entity& lhs, const entity& rhs) {
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

void
ggg::Hierarchy::add(const entity& ent, const std::string& filename) {
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
		throw std::invalid_argument("entity with the same name/uid already exists");
	}
	if (::getpwnam(ent.name().data())) {
		throw std::invalid_argument("conflicting system user");
	}
	this->append_entity(ent, filename);
}

void
ggg::Hierarchy::append_entity(const entity& ent, const std::string& filename) {
	sys::path dest(this->_root, filename);
	if (this->verbose()) {
		std::clog
			<< "appending " << ent.name() << ':' << ent.id()
			<< " to " << dest
			<< std::endl;
	}
	bits::append(ent, dest, "unable to add entity");
}
