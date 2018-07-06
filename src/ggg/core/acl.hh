#ifndef GGG_CORE_ACL_HH
#define GGG_CORE_ACL_HH

#include <acl/libacl.h>
#include <sys/acl.h>
#include <sys/types.h>

#include <cassert>
#include <cstring>
#include <iterator>
#include <ostream>
#include <type_traits>
#include <vector>

#include <unistdx/base/check>
#include <unistdx/fs/file_mode>

namespace ggg {

	namespace acl {

		enum acl_type: ::acl_type_t {
			access_acl = ACL_TYPE_ACCESS,
			default_acl = ACL_TYPE_DEFAULT
		};

		typedef ::acl_t acl_pointer;
		typedef ::acl_entry_t entry_pointer;
		typedef ::acl_permset_t permission_set_pointer;
		typedef typename std::remove_pointer<entry_pointer>::type entry_type;

		enum struct tag_type: ::acl_tag_t {
			undefined = ACL_UNDEFINED_TAG,
			owner_user = ACL_USER_OBJ,
			user = ACL_USER,
			group = ACL_GROUP,
			owner_group = ACL_GROUP_OBJ,
			mask = ACL_MASK,
			other = ACL_OTHER
		};

		enum struct permission_type: ::acl_perm_t {
			read = ACL_READ,
			write = ACL_WRITE,
			execute = ACL_EXECUTE,
			read_write = ACL_READ | ACL_WRITE,
		};

		inline permission_type
		operator|(permission_type lhs, permission_type rhs) {
			typedef ::acl_perm_t tp;
			return permission_type(tp(lhs) | tp(rhs));
		}

		inline permission_type
		user_permissions(sys::file_mode m) {
			::acl_perm_t perms = 0;
			if (m.user() & sys::file_mode::user_r) {
				perms |= ACL_READ;
			}
			if (m.user() & sys::file_mode::user_w) {
				perms |= ACL_WRITE;
			}
			if (m.user() & sys::file_mode::user_x) {
				perms |= ACL_EXECUTE;
			}
			return permission_type(perms);
		}

		inline permission_type
		group_permissions(sys::file_mode m) {
			::acl_perm_t perms = 0;
			if (m.group() & sys::file_mode::group_r) {
				perms |= ACL_READ;
			}
			if (m.group() & sys::file_mode::group_w) {
				perms |= ACL_WRITE;
			}
			if (m.group() & sys::file_mode::group_x) {
				perms |= ACL_EXECUTE;
			}
			return permission_type(perms);
		}

		inline permission_type
		other_permissions(sys::file_mode m) {
			::acl_perm_t perms = 0;
			if (m.other() & sys::file_mode::other_r) {
				perms |= ACL_READ;
			}
			if (m.other() & sys::file_mode::other_w) {
				perms |= ACL_WRITE;
			}
			if (m.other() & sys::file_mode::other_x) {
				perms |= ACL_EXECUTE;
			}
			return permission_type(perms);
		}

		inline std::vector<std::string>
		to_sorted_lines(const char* rhs) {
			std::stringstream str;
			str << rhs;
			std::string line;
			std::vector<std::string> lines;
			while (std::getline(str, line, '\n')) {
				lines.push_back(line);
			}
			std::sort(lines.begin(), lines.end());
			return lines;
		}

		inline bool
		equal_sort_lines(const char* str1, const char* str2) {
			return to_sorted_lines(str1) == to_sorted_lines(str2);
		}

		class permission_set {

		private:
			permission_set_pointer _ptr = nullptr;

		public:

			permission_set() = default;

			permission_set(const permission_set&) = default;

			permission_set(permission_set&&) = default;

			~permission_set() = default;

			permission_set& operator=(const permission_set&) = default;

			permission_set& operator=(permission_set&&) = default;

			inline permission_set_pointer
			get() noexcept {
				return this->_ptr;
			}

			inline void
			add(permission_type rhs) {
				UNISTDX_CHECK(::acl_add_perm(
					this->_ptr,
					static_cast<::acl_perm_t>(rhs)
				));
			}

			inline void
			remove(permission_type rhs) {
				UNISTDX_CHECK(::acl_delete_perm(
					this->_ptr,
					static_cast<::acl_perm_t>(rhs)
				));
			}

			inline void
			clear() {
				UNISTDX_CHECK(::acl_clear_perms(this->_ptr));
			}

			friend class entry;

		};

		class entry {

		private:
			entry_pointer _entry = nullptr;

		public:

			entry() = default;

			inline explicit
			entry(entry_pointer ptr):
			_entry(ptr)
			{}

			inline
			entry(const entry& rhs) {
				if (rhs._entry) {
					UNISTDX_CHECK(::acl_copy_entry(this->_entry, rhs._entry));
				}
			}

			inline
			entry(entry&& rhs) {
				this->_entry = rhs._entry;
				rhs._entry = nullptr;
			}

			~entry() = default;

			inline entry&
			operator=(const entry& rhs) {
				if (rhs._entry) {
					UNISTDX_CHECK(::acl_copy_entry(this->_entry, rhs._entry));
				}
				return *this;
			}

			inline entry&
			operator=(entry&& rhs) {
				this->_entry = rhs._entry;
				return *this;
			}

			inline bool
			operator==(const entry& rhs) const noexcept {
				return this->_entry == rhs._entry;
			}

			inline bool
			operator!=(const entry& rhs) const noexcept {
				return !this->operator==(rhs);
			}

			inline entry_pointer
			get() noexcept {
				return this->_entry;
			}

			inline permission_set
			permissions() {
				permission_set s;
				UNISTDX_CHECK(::acl_get_permset(this->_entry, &s._ptr));
				return s;
			}

			inline void
			permissions(permission_set rhs) {
				UNISTDX_CHECK(::acl_set_permset(this->_entry, rhs.get()));
			}

			inline tag_type
			tag() {
				::acl_tag_t t;
				UNISTDX_CHECK(::acl_get_tag_type(this->_entry, &t));
				return tag_type(t);
			}

			inline void
			tag(tag_type t) {
				UNISTDX_CHECK(::acl_set_tag_type(
					this->_entry,
					static_cast<::acl_tag_t>(t)
				));
			}

			inline bool
			is_user() {
				return this->tag() == tag_type::user;
			}

			inline bool
			is_group() {
				return this->tag() == tag_type::group;
			}

			inline bool
			is_owner_user() {
				return this->tag() == tag_type::owner_user;
			}

			inline bool
			is_owner_group() {
				return this->tag() == tag_type::owner_group;
			}

			inline bool
			is_mask() {
				return this->tag() == tag_type::mask;
			}

			inline bool
			is_other() {
				return this->tag() == tag_type::other;
			}

			inline bool
			is_undefined() {
				return this->tag() == tag_type::undefined;
			}

			inline sys::uid_type
			user() {
				void* ret = ::acl_get_qualifier(this->_entry);
				UNISTDX_CHECK2(ret, nullptr);
				return *reinterpret_cast<sys::uid_type*>(ret);
			}

			inline void
			user(sys::uid_type uid) {
				UNISTDX_CHECK(::acl_set_qualifier(this->_entry, &uid));
			}

			inline sys::uid_type
			group() {
				void* ret = ::acl_get_qualifier(this->_entry);
				UNISTDX_CHECK2(ret, nullptr);
				return *reinterpret_cast<sys::gid_type*>(ret);
			}

			inline void
			group(sys::gid_type gid) {
				UNISTDX_CHECK(::acl_set_qualifier(this->_entry, &gid));
			}

			friend class access_control_list;
			friend class entry_iterator;

		};

		class entry_iterator:
			public std::iterator<std::input_iterator_tag, entry> {

		private:
			typedef typename std::iterator<std::input_iterator_tag, entry>
				base_type;

		public:
			using typename base_type::iterator_category;
			using typename base_type::value_type;
			using typename base_type::pointer;
			using typename base_type::reference;
			using typename base_type::difference_type;

		private:
			typedef const entry* const_pointer;
			typedef const entry& const_reference;

		private:
			acl_pointer _acl = nullptr;
			entry _entry;

		public:

			inline explicit
			entry_iterator(acl_pointer acl):
			_acl(acl) {
				int ret = ::acl_get_entry(
					this->_acl,
					ACL_FIRST_ENTRY,
					&this->_entry._entry
				);
				UNISTDX_CHECK(ret);
				assert(
					(ret == 0 && !this->_entry._entry) ||
					(ret == 1 && this->_entry._entry)
				);
			}

			inline
			entry_iterator() noexcept = default;

			inline
			~entry_iterator() {
				this->_entry._entry = nullptr;
			}

			/// Copy-constructor.
			inline
			entry_iterator(const entry_iterator& rhs):
			_acl(rhs._acl),
			_entry(rhs._entry) {}

			/// Assignment.
			inline entry_iterator&
			operator=(const entry_iterator&) = default;

			inline bool
			operator==(const entry_iterator& rhs) const noexcept {
				return this->_entry == rhs._entry;
			}

			inline bool
			operator!=(const entry_iterator& rhs) const noexcept {
				return !this->operator==(rhs);
			}

			/// Dereference.
			inline reference
			operator*() noexcept {
				return this->_entry;
			}

			/// Dereference.
			inline const_reference
			operator*() const noexcept {
				return this->_entry;
			}

			/// Access object by pointer.
			inline pointer
			operator->() noexcept {
				return &this->_entry;
			}

			/// Access object by pointer.
			inline const_pointer
			operator->() const noexcept {
				return &this->_entry;
			}

			/// Increment.
			inline entry_iterator&
			operator++() noexcept {
				this->next(); return *this;
			}

			/// Post-increment.
			inline entry_iterator
			operator++(int) noexcept {
				entry_iterator tmp(*this); this->next(); return tmp;
			}

		private:

			inline void
			next() noexcept {
				int ret = ::acl_get_entry(
					this->_acl,
					ACL_NEXT_ENTRY,
					&this->_entry._entry
				);
				UNISTDX_CHECK(ret);
				if (ret == 0) {
					this->_entry._entry = nullptr;
				}
				assert(
					(ret == 0 && !this->_entry._entry) ||
					(ret == 1 && this->_entry._entry)
				);
			}

		};

		class access_control_list {

		public:
			typedef ::acl_t pointer;

		private:
			pointer _acl = nullptr;

		public:

			access_control_list() = default;

			inline explicit
			access_control_list(int nentries):
			_acl(::acl_init(nentries)) {
				UNISTDX_CHECK2(this->_acl, nullptr);
			}

			inline
			access_control_list(const access_control_list& rhs) {
				if (rhs._acl) {
					this->_acl = ::acl_dup(rhs._acl);
					UNISTDX_CHECK2(this->_acl, nullptr);
				}
			}

			inline
			access_control_list(const char* filename, acl_type tp):
			_acl(::acl_get_file(filename, static_cast<::acl_type_t>(tp)))
			{
				UNISTDX_CHECK2(this->_acl, nullptr);
			}

			inline explicit
			access_control_list(sys::file_mode m):
			_acl(::acl_from_mode(m))
			{
				UNISTDX_CHECK2(this->_acl, nullptr);
			}

			inline
			~access_control_list() {
				UNISTDX_CHECK(::acl_free(this->_acl));
			}

			access_control_list&
			operator=(access_control_list rhs) {
				std::swap(this->_acl, rhs._acl);
				return *this;
			}

			explicit inline
			operator bool() const noexcept {
				return this->_acl;
			}

			inline bool
			operator!() const noexcept {
				return !this->operator bool();
			}

			inline void
			validate() {
				UNISTDX_CHECK(::acl_valid(this->_acl));
			}

			inline void
			add_mask() {
				UNISTDX_CHECK(::acl_calc_mask(&this->_acl));
			}

			inline pointer
			get() noexcept {
				return this->_acl;
			}

			inline entry_iterator
			begin() {
				return entry_iterator(this->_acl);
			}

			inline entry_iterator
			end() {
				return entry_iterator();
			}

			inline size_t
			size() {
				return std::distance(this->begin(), this->end());
			}

			inline ssize_t
			size_in_bytes() {
				ssize_t ret = 0;
				UNISTDX_CHECK(::acl_size(this->_acl));
				return ret;
			}

			inline void
			remove(entry& ent) {
				UNISTDX_CHECK(::acl_delete_entry(this->_acl, ent.get()));
			}

			inline void
			clear() {
				for (entry& ent : *this) {
					this->remove(ent);
				}
			}

			inline entry
			add() {
				entry ent;
				UNISTDX_CHECK(::acl_create_entry(&this->_acl, &ent._entry));
				return ent;
			}

			inline void
			add(tag_type tag, permission_type perms) {
				entry e = this->add();
				e.tag(tag);
				e.permissions().add(perms);
			}

			inline void
			add_user(sys::uid_type uid, permission_type perms) {
				entry e = this->add();
				e.tag(tag_type::user);
				e.user(uid);
				e.permissions().add(perms);
			}

			inline void
			add_group(sys::gid_type gid, permission_type perms) {
				entry e = this->add();
				e.tag(tag_type::group);
				e.group(gid);
				e.permissions().add(perms);
			}

			inline void
			mode(sys::file_mode m) {
				this->add(tag_type::owner_user, user_permissions(m));
				this->add(tag_type::owner_group, group_permissions(m));
				this->add(tag_type::other, other_permissions(m));
			}

			inline void
			read(const char* filename, acl_type tp) {
				if (this->_acl) {
					UNISTDX_CHECK(::acl_free(this->_acl));
				}
				this->_acl = ::acl_get_file(filename, static_cast<::acl_type_t>(tp));
				UNISTDX_CHECK2(this->_acl, nullptr);
			}

			inline void
			write(const char* filename, acl_type tp) const {
				UNISTDX_CHECK(::acl_set_file(
					filename,
					static_cast<::acl_type_t>(tp),
					this->_acl
				));
			}

			inline bool
			operator==(const access_control_list& rhs) {
				if (!this->_acl && !rhs._acl) {
					return true;
				}
				ssize_t len1 = 0;
				ssize_t len2 = 0;
				char* str1 = ::acl_to_text(this->_acl, &len1);
				char* str2 = ::acl_to_text(rhs._acl, &len2);
				try {
					UNISTDX_CHECK2(str1, nullptr);
					UNISTDX_CHECK2(str2, nullptr);
				} catch (const sys::bad_call& err) {
					if (err.errc() == std::errc::invalid_argument) {
						return false;
					}
					throw;
				}
				return len1 == len2 &&
					(std::strncmp(str1, str2, len1) == 0 ||
					 equal_sort_lines(str1, str2));
			}

			inline bool
			operator!=(const access_control_list& rhs) {
				return !this->operator==(rhs);
			}

			friend std::ostream&
			operator<<(std::ostream& out, const access_control_list& rhs) {
				if (!rhs._acl) {
					return out << "<nullptr>";
				}
				char* str = ::acl_to_text(rhs._acl, nullptr);
				UNISTDX_CHECK2(str, nullptr);
				try {
					out << str;
				} catch (...) {
					UNISTDX_CHECK(::acl_free(str));
				}
				return out;
			}

		};

		inline void
		remove_default_acl(const char* filename) {
			UNISTDX_CHECK(::acl_delete_def_file(filename));
		}

	}

}

#endif // vim:filetype=cpp
