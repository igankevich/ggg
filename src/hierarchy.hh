#ifndef HIERARCHY_HH
#define HIERARCHY_HH

#include "entity.hh"

#include <stdx/iterator.hh>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <map>
#include <deque>
#include <sstream>

#include "field_iterator.hh"

namespace legion {

	template<class Container, class Pred>
	inline void
	erase_if(Container& cont, Pred pred) {
		for (auto it=cont.begin(); it!=cont.end(); ) {
			if (pred(*it)) it = cont.erase(it);
			else ++it;
		}
	};

	typedef std::pair<const entity,std::set<std::string>> entity_pair;

	std::ostream&
	operator<<(std::ostream& out, const entity_pair& rhs);

	class Hierarchy {
		typedef std::map<entity,std::set<std::string>> map_type;

		map_type _entities;
		map_type _links;
		sys::canonical_path _root;
		sys::uid_type _minuid = 1000;
		size_t _maxlinks = 100;

	public:
		typedef stdx::field_iterator<map_type,0> iterator;

		Hierarchy() = default;

		inline explicit
		Hierarchy(sys::path root):
		_root(root)
		{}

		void
		open(const sys::path& root) {
			_root = sys::canonical_path(root);
		}

		void read();
		void print();

		inline void
		clear() {
			_entities.clear();
			_links.clear();
		}

		iterator
		begin() {
			return _entities.begin();
		}

		iterator
		end() {
			return _entities.end();
		}

		iterator
		find_by_uid(sys::uid_type uid) {
			return std::find_if(
				begin(),
				end(),
				[uid] (const entity& ent) {
					return ent.id() == uid;
				}
			);
		}

		iterator
		find_by_name(const char* name) {
			return std::find_if(
				begin(),
				end(),
				[name] (const entity& ent) {
					return std::strcmp(ent.name(), name);
				}
			);
		}


	private:
		void
		process_links();

		void
		filter_entities();

		void
		process_entry(const sys::pathentry& entry, bool& success);

		inline void
		add_entity(const entity& ent, const sys::canonical_path& filepath) {
			add(_entities, ent, filepath);
		}

		inline void
		add_link(const entity& ent, const sys::canonical_path& filepath) {
			add(_links, ent, filepath);
		}

		void
		add(
			map_type& container,
			const entity& ent,
			const sys::canonical_path& filepath
		);

	};

}

#endif // HIERARCHY_HH
