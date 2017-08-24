#ifndef HIERARCHY_HH
#define HIERARCHY_HH

#include "entity.hh"
#include "group.hh"

#include <algorithm>
#include <iterator>
#include <set>
#include <map>
#include <deque>
#include <sstream>
#include <unistdx/fs/canonical_path>
#include <unistdx/fs/pathentry>

#include "bits/field_iterator.hh"

namespace ggg {

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
		typedef std::set<group> group_container_type;

		map_type _entities;
		map_type _links;
		std::set<group> _groups;
		sys::canonical_path _root;
		sys::uid_type _minuid = 1000;
		sys::gid_type _mingid = 1000;
		size_t _maxlinks = 100;
		bool _isopen = false;
		bool _verbose = false;

	public:
		typedef stdx::field_iterator<map_type::iterator,0> iterator;
		typedef stdx::field_iterator<map_type::const_iterator,0> const_iterator;
		typedef group_container_type::iterator group_iterator;
		typedef group_container_type::const_iterator group_const_iterator;

		Hierarchy() = default;

		inline explicit
		Hierarchy(sys::path root)
		{ open(root); }

		void
		open(const sys::path& root) {
			_root = sys::canonical_path(root);
			read();
		}

		bool
		is_open() const noexcept {
			return _isopen;
		}

		void
		ensure_open(const char* root) {
			if (!is_open()) {
				open(sys::path(root));
			}
		}

		inline void
		clear() {
			_entities.clear();
			_links.clear();
			_isopen = false;
		}

		inline const sys::canonical_path&
		root() const noexcept {
			return this->_root;
		}

		iterator
		begin() {
			return _entities.begin();
		}

		iterator
		end() {
			return _entities.end();
		}

		const_iterator
		begin() const {
			return _entities.begin();
		}

		const_iterator
		end() const {
			return _entities.end();
		}

		const_iterator
		cbegin() const {
			return _entities.begin();
		}

		const_iterator
		cend() const {
			return _entities.end();
		}

		const_iterator
		find_by_uid(sys::uid_type uid) const {
			return std::find_if(
				begin(),
				end(),
				[uid] (const entity& ent) {
					return ent.id() == uid;
				}
			);
		}

		const_iterator
		find_by_name(const char* name) const {
			return std::find_if(
				begin(),
				end(),
				[name] (const entity& ent) {
					return ent.name() == name;
				}
			);
		}

		const_iterator
		find_link_by_name(const char* name) const {
			return std::find_if(
				const_iterator(this->_links.begin()),
				const_iterator(this->_links.end()),
				[name] (const entity& ent) {
					return ent.name() == name;
				}
			);
		}

		void
		erase(const char* name);

		void
		update(const entity& ent);

		void
		add(const entity& ent, const std::string& filename);

		entity
		generate(const char* name);

		sys::uid_type next_uid() const;

		group_iterator
		group_begin() {
			return _groups.begin();
		}

		group_iterator
		group_end() {
			return _groups.end();
		}

		group_const_iterator
		group_begin() const {
			return _groups.begin();
		}

		group_const_iterator
		group_end() const {
			return _groups.end();
		}

		group_const_iterator
		find_group_by_gid(sys::gid_type gid) const {
			return std::find_if(
				_groups.begin(),
				_groups.end(),
				[gid] (const group& grp) {
					return grp.id() == gid;
				}
			);
		}

		group_const_iterator
		find_group_by_name(const char* name) const {
			return _groups.find(group(name));
		}

		void
		setminuid(sys::uid_type rhs) noexcept {
			_minuid = rhs;
		}

		inline void
		verbose(bool rhs) noexcept {
			this->_verbose = rhs;
		}

		inline bool
		verbose() const noexcept {
			return this->_verbose;
		}

		friend std::ostream&
		operator<<(std::ostream& out, const Hierarchy& rhs);

	private:
		void
		read();

		void
		process_links();

		void
		filter_entities();

		void
		generate_groups();

		void
		filter_groups();

		void
		process_entry(const sys::pathentry& entry, bool& success);

		inline void
		add_entity(const entity& ent, const sys::canonical_path& filepath) {
			this->add(this->_entities, ent, filepath);
		}

		inline void
		add_link(const entity& ent, const sys::canonical_path& filepath) {
			this->add(this->_links, ent, filepath);
		}

		void
		add(
			map_type& container,
			const entity& ent,
			const sys::canonical_path& filepath
		);

		void
		erase_regular(const entity& ent);

		void
		erase_link(const entity& ent);

		void
		update_regular(const entity& ent, sys::path origin);

		void
		validate_entity(const entity& ent);

		void
		append_entity(const entity& ent, const std::string& filename);

	};

	std::ostream&
	operator<<(std::ostream& out, const Hierarchy& rhs);

}

#endif // HIERARCHY_HH
