#include "hierarchy.hh"
#include <stdx/iterator.hh>

std::ostream&
legion::operator<<(std::ostream& out, const entity_pair& rhs) {
	out << rhs.first << ':';
	std::copy(
		rhs.second.begin(),
		rhs.second.end(),
		stdx::intersperse_iterator<std::string>(out, ",")
	);
	return out;
}

void
legion::Hierarchy::read() {
	sys::dirtree tree;
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
					process_entry(entry, success);
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
}

std::ostream&
legion::operator<<(std::ostream& out, const Hierarchy& rhs) {
	std::cout << "\nUsers:\n";
	std::copy(
		rhs._entities.begin(),
		rhs._entities.end(),
		std::ostream_iterator<entity_pair>(std::cout, "\n")
	);
	std::cout << "\nGroups:\n";
	std::copy(
		rhs._groups.begin(),
		rhs._groups.end(),
		std::ostream_iterator<group>(std::cout, "\n")
	);
	return out;
}

void
legion::Hierarchy::process_links() {
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
legion::Hierarchy::filter_entities() {
	erase_if(
		_entities,
		[this] (const entity_pair& rhs) {
			return !rhs.first.has_id() || rhs.first.id() < _minuid;
		}
	);
}

void
legion::Hierarchy::generate_groups() {
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
legion::Hierarchy::filter_groups() {
	// remove empty groups
	erase_if(
		_groups,
		[] (const group& rhs) {
			return rhs.members().empty();
		}
	);
	// remove duplicate entities
	for (const group& grp : _groups) {
		_entities.erase(entity(grp.name().data()));
	}
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
legion::Hierarchy::process_entry(const sys::pathentry& entry, bool& success) {
	sys::canonical_path filepath(entry.getpath());
	if (filepath.is_relative_to(_root)) {
		if (entry.type() == sys::file_type::regular) {
			std::ifstream in(filepath);
			if (in.is_open()) {
				entity ent;
				while (in >> ent) {
					add_entity(ent, filepath);
				}
				add_entity(entity(entry.name()), filepath.dirname());
				success = true;
			}
		} else if (entry.type() == sys::file_type::symbolic_link) {
			entity ent(entry.name());
			add_link(ent, sys::canonical_path(entry.dirname()));
		}
	}
}

void
legion::Hierarchy::add(
	map_type& container,
	const entity& ent,
	const sys::canonical_path& filepath
) {
	const std::string& filestr = filepath.to_string();
	std::string dirs(
		filestr,
		std::min(filestr.size(), _root.to_string().size() + 1)
	);
	std::stringstream tmp;
	tmp << dirs;
	std::set<std::string>& principals = container[ent];
	std::string group;
	while (std::getline(tmp, group, sys::path::separator)) {
		principals.emplace(group);
	}
}
