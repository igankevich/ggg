#ifndef GROUP_HH
#define GROUP_HH

#include <grp.h>
#include <unistd.h>

#include <string>
#include <unordered_set>

#include <unistdx/net/bstream>

#include <sqlitex/rstream.hh>

namespace sys {
	typedef ::gid_t gid_type;
}

namespace ggg {

	template <class Ch>
	class basic_group {

	public:
		typedef Ch char_type;
		typedef std::basic_string<Ch> string_type;
		typedef std::unordered_set<string_type> container_type;

	private:
		string_type _name;
		sys::gid_type _gid;
		mutable container_type _members;

	public:

		inline explicit
		basic_group(const string_type& name, sys::gid_type gid):
		_name(name), _gid(gid) {}

		inline explicit
		basic_group(
			const string_type& name,
			sys::gid_type gid,
			const container_type& members
		):
		_name(name),
		_gid(gid),
		_members(members)
		{}

		inline explicit
		basic_group(const string_type& name):
		_name(name)
		{}

		basic_group() = default;
		basic_group(const basic_group& rhs) = default;
		basic_group(basic_group&& rhs) = default;
		~basic_group() = default;
		basic_group& operator=(const basic_group& rhs) = default;
		basic_group& operator=(basic_group&& rhs) = default;

		inline bool
		has_id() const noexcept {
			return this->_gid != sys::gid_type(-1);
		}

		inline bool
		operator==(const basic_group& rhs) const noexcept {
			return name() == rhs.name();
		}

		inline bool
		operator!=(const basic_group& rhs) const noexcept {
			return !operator==(rhs);
		}

		inline bool
		operator<(const basic_group& rhs) const noexcept {
			return name() < rhs.name();
		}

		template <class X>
		friend std::basic_ostream<X>&
		operator<<(std::basic_ostream<X>& out, const basic_group<X>& rhs);

		template <class X>
		friend sys::basic_bstream<X>&
		operator<<(sys::basic_bstream<X>& out, const basic_group<X>& rhs);

		template <class X>
		friend sys::basic_bstream<X>&
		operator>>(sys::basic_bstream<X>& in, basic_group<X>& rhs);

		friend sqlite::rstream&
		operator>>(sqlite::rstream& in, basic_group<char>& rhs);

		inline sys::gid_type
		id() const noexcept {
			return this->_gid;
		}

		inline const string_type&
		name() const noexcept {
			return this->_name;
		}

		inline container_type&
		members() const noexcept {
			return this->_members;
		}

		inline void
		members(container_type&& rhs) {
			this->_members = std::move(rhs);
		}

		inline void
		push(const string_type& member) const {
			this->_members.emplace(member);
		}

		inline void
		erase(const string_type& member) const {
			this->_members.erase(member);
		}

		inline void
		clear() const {
			this->_members.clear();
		}

		inline void
		swap(basic_group& rhs) {
			using std::swap;
			swap(this->_name, rhs._name);
			swap(this->_gid, rhs._gid);
			swap(this->_members, rhs._members);
		}

	};

	template <class Ch>
	inline void
	swap(basic_group<Ch>& lhs, basic_group<Ch>& rhs) {
		lhs.swap(rhs);
	}

	template <class Ch>
	std::basic_ostream<Ch>&
	operator<<(std::basic_ostream<Ch>& out, const basic_group<Ch>& rhs);

	template <class Ch>
	sys::basic_bstream<Ch>&
	operator<<(sys::basic_bstream<Ch>& out, const basic_group<Ch>& rhs);

	template <class Ch>
	sys::basic_bstream<Ch>&
	operator>>(sys::basic_bstream<Ch>& in, basic_group<Ch>& rhs);

	sqlite::rstream&
	operator>>(sqlite::rstream& in, basic_group<char>& rhs);

	typedef basic_group<char> group;

}

namespace std {

	template<class Ch>
	struct hash<ggg::basic_group<Ch>>: public hash<sys::gid_type> {

		typedef size_t result_type;
		typedef ggg::basic_group<Ch> argument_type;

		inline result_type
		operator()(const argument_type& rhs) const noexcept {
			return hash<sys::gid_type>::operator()(rhs.id());
		}

	};

}

#endif // GROUP_HH
