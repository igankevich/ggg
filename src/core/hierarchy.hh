#ifndef HIERARCHY_HH
#define HIERARCHY_HH

#include "entity.hh"
#include "group.hh"

#include <algorithm>
#include <deque>
#include <iterator>
#include <map>
#include <set>
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

	template <class Ch>
	using entity_pair =
		std::pair<const basic_entity<Ch>, std::set<std::basic_string<Ch>>>;

	template <class Ch>
	std::basic_ostream<Ch>&
	operator<<(std::basic_ostream<Ch>& out, const entity_pair<Ch>& rhs);

	template <class Ch>
	class basic_hierarchy {

	private:
		typedef Ch char_type;
		typedef std::basic_string<Ch> string_type;
		typedef std::char_traits<Ch> traits_type;
		typedef basic_entity<Ch> entity_type;
		typedef basic_group<Ch> group_type;
		typedef entity_pair<Ch> entity_pair_type;
		typedef std::set<string_type> entity_set_type;
		typedef std::map<entity_type,entity_set_type> map_type;
		typedef std::set<group_type> group_container_type;
		typedef sys::canonical_path canonical_path_type;
		typedef typename entity_type::path_type path_type;

	private:
		map_type _entities;
		map_type _links;
		std::set<group_type> _groups;
		canonical_path_type _root;
		sys::uid_type _minuid = 1000;
		sys::gid_type _mingid = 1000;
		size_t _maxlinks = 100;
		bool _isopen = false;
		bool _verbose = false;

	public:
		typedef stdx::field_iterator<typename map_type::iterator,0>
			iterator;
		typedef stdx::field_iterator<typename map_type::const_iterator,0>
			const_iterator;
		typedef typename group_container_type::iterator
			group_iterator;
		typedef typename group_container_type::const_iterator
			group_const_iterator;
		typedef typename group_type::container_type
			group_member_container_type;

		basic_hierarchy() = default;

		inline explicit
		basic_hierarchy(const char* root)
		{ this->open(root); }

		inline void
		open(const char* root) {
			this->_root = canonical_path_type(root);
			this->read();
		}

		inline bool
		is_open() const noexcept {
			return _isopen;
		}

		inline void
		ensure_open(const char* root) {
			if (!is_open()) {
				this->open(root);
			}
		}

		inline void
		clear() {
			this->_entities.clear();
			this->_links.clear();
			this->_isopen = false;
		}

		inline const canonical_path_type&
		root() const noexcept {
			return this->_root;
		}

		inline iterator
		begin() {
			return this->_entities.begin();
		}

		inline iterator
		end() {
			return this->_entities.end();
		}

		inline const_iterator
		begin() const {
			return this->_entities.begin();
		}

		inline const_iterator
		end() const {
			return this->_entities.end();
		}

		inline const_iterator
		cbegin() const {
			return this->_entities.begin();
		}

		inline const_iterator
		cend() const {
			return this->_entities.end();
		}

		const_iterator
		find_by_uid(sys::uid_type uid) const {
			return std::find_if(
				begin(),
				end(),
				[uid] (const entity_type& ent) {
					return ent.id() == uid;
				}
			);
		}

		const_iterator
		find_by_name(const char_type* name) const {
			return std::find_if(
				begin(),
				end(),
				[name] (const entity_type& ent) {
					return ent.name() == name;
				}
			);
		}

		const_iterator
		find_link_by_name(const char_type* name) const {
			return std::find_if(
				const_iterator(this->_links.begin()),
				const_iterator(this->_links.end()),
				[name] (const entity_type& ent) {
					return ent.name() == name;
				}
			);
		}

		group_member_container_type
		find_supplementary_groups(const string_type& name) const {
			group_member_container_type result;
			for (const group_type& g : this->_groups) {
				if (g.members().find(name) != g.members().end()) {
					result.emplace(g.name());
				}
			}
			return result;
		}

		void
		erase(const char_type* name);

		void
		update(const entity_type& ent);

		void
		add(const entity_type& ent, const std::string& filename);

		entity_type
		generate(const char_type* name);

		template <class Iterator, class Result>
		void
		generate(Iterator first, Iterator last, Result result) {
			sys::uid_type id = this->next_uid();
			while (first != last) {
				*result = entity_type(first->data(), id, id);
				++first;
				++id;
			}
		}

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
				[gid] (const group_type& grp) {
					return grp.id() == gid;
				}
			);
		}

		group_const_iterator
		find_group_by_name(const char_type* name) const {
			return this->_groups.find(group_type(name));
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

		template <class X>
		friend std::basic_ostream<X>&
		operator<<(std::basic_ostream<X>& out, const basic_hierarchy<X>& rhs);

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
		process_entry(
			const sys::pathentry& entry,
			entity::wcvt_type& cv,
			bool& success
		);

		inline void
		add_entity(
			const entity_type& ent,
			const canonical_path_type& filepath,
			entity::wcvt_type& cv
		) {
			this->add(this->_entities, ent, filepath, cv);
		}

		inline void
		add_link(
			const entity_type& ent,
			const canonical_path_type& filepath,
			entity::wcvt_type& cv
		) {
			this->add(this->_links, ent, filepath, cv);
		}

		void
		add(
			map_type& container,
			const entity_type& ent,
			const canonical_path_type& filepath,
			entity::wcvt_type& cv
		);

		void
		erase_regular(const entity_type& ent);

		void
		erase_link(const entity_type& ent);

		void
		update_regular(const entity_type& ent, path_type origin);

		void
		validate_entity(const entity_type& ent);

		void
		append_entity(const entity_type& ent, const std::string& filename);

	};

	template <class Ch>
	std::basic_ostream<Ch>&
	operator<<(std::basic_ostream<Ch>& out, const basic_hierarchy<Ch>& rhs);

	typedef basic_hierarchy<char> Hierarchy;

}

#endif // HIERARCHY_HH
