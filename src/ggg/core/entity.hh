#ifndef ENTITY_HH
#define ENTITY_HH

#include <codecvt>
#include <functional>
#include <istream>
#include <locale>
#include <string>

#include <unistdx/fs/path>
#include <unistdx/net/bstream>
#include <unistdx/util/user>

#include <sqlitex/statement.hh>

#include <ggg/core/entity_format.hh>
#include <ggg/core/eq_traits.hh>
#include <ggg/core/guile_traits.hh>

namespace ggg {

	constexpr const sys::uid_type bad_uid = -1;
	constexpr const sys::gid_type bad_gid = -1;

	template <class Ch>
	class basic_entity {

	public:
		typedef Ch char_type;
		typedef std::char_traits<Ch> traits_type;
		typedef std::basic_string<Ch, traits_type> string_type;
		typedef sys::path path_type;

	private:
		typedef std::basic_ostream<Ch, traits_type> ostream_type;

	public:
		typedef const size_t columns_type[7];
		static constexpr const char_type delimiter = ':';

	private:
		string_type _name;
		string_type _password;
		string_type _realname;
		string_type _homedir;
		string_type _shell;
		sys::uid_type _uid = -1;
		sys::gid_type _gid = -1;
		string_type _parent;

	public:

		inline explicit
		basic_entity(const char_type* name):
		_name(name)
		{}

		inline explicit
		basic_entity(const char_type* name, sys::uid_type uid, sys::gid_type gid):
		_name(name),
		_homedir(),
		_shell(),
		_uid(uid),
		_gid(gid)
		{ this->_homedir.append(_name); }

		basic_entity() = default;
		basic_entity(const basic_entity& rhs) = default;
		basic_entity(basic_entity&& rhs) = default;
		~basic_entity() = default;

		basic_entity&
		operator=(const basic_entity& rhs) = default;

		template <class X>
		friend void
		assign(basic_entity<X>& lhs, const struct ::passwd& rhs);

		inline bool
		has_id() const noexcept {
			return this->_uid != sys::uid_type(-1);
		}

		inline bool
		has_gid() const noexcept {
			return this->_gid != sys::gid_type(-1);
		}

		inline void
		set_uid(sys::uid_type rhs) noexcept {
			this->_uid = rhs;
		}

		inline void
		set_gid(sys::gid_type rhs) noexcept {
			this->_gid = rhs;
		}

		inline bool
		operator==(const basic_entity& rhs) const noexcept {
			return name() == rhs.name();
		}

		inline bool
		operator!=(const basic_entity& rhs) const noexcept {
			return !operator==(rhs);
		}

		inline bool
		operator<(const basic_entity& rhs) const noexcept {
			return name() < rhs.name();
		}

		template <class X>
		friend std::basic_istream<X>&
		operator>>(std::basic_istream<X>& in, basic_entity<X>& rhs);

		template <class X>
		friend std::basic_ostream<X>&
		operator<<(std::basic_ostream<X>& out, const basic_entity<X>& rhs);

		template <class X>
		friend sys::basic_bstream<X>&
		operator>>(sys::basic_bstream<X>& in, basic_entity<X>& rhs);

		template <class X>
		friend sys::basic_bstream<X>&
		operator<<(sys::basic_bstream<X>& out, const basic_entity<X>& rhs);

		friend void
		operator>>(const sqlite::statement& in, basic_entity<char>& rhs);

		inline sys::uid_type
		id() const noexcept {
			return this->_uid;
		}

		inline void
		id(sys::uid_type uid) noexcept {
			this->_uid = uid;
		}

		inline sys::gid_type
		gid() const noexcept {
			return this->_gid;
		}

		inline const string_type&
		name() const noexcept {
			return this->_name;
		}

		inline bool
		has_name() const noexcept {
			return !this->_name.empty();
		}

		bool
		has_valid_name() const noexcept;

		inline const string_type&
		password() const noexcept {
			return this->_password;
		}

		inline const string_type&
		real_name() const noexcept {
			return this->_realname;
		}

		inline const string_type&
		home() const noexcept {
			return this->_homedir;
		}

		inline void
		home(const string_type& rhs) {
			this->_homedir = rhs;
		}

		inline const string_type&
		shell() const noexcept {
			return this->_shell;
		}

		inline void
		shell(const string_type& rhs) {
			this->_shell = rhs;
		}

		inline const string_type&
		parent() const noexcept {
			return this->_parent;
		}

		inline void
		parent(const string_type& rhs) {
			this->_parent = rhs;
		}

		inline bool
		has_parent() const noexcept {
			return !this->_parent.empty();
		}

		inline bool
		has_valid_parent() const noexcept {
			return this->has_parent() && this->parent() != this->name();
		}

		void clear();

		void
		merge(const basic_entity& rhs);

		friend struct Entity_header<basic_entity>;
		friend struct Guile_traits<basic_entity>;

	};

	template <class Ch>
	void
	assign(basic_entity<Ch>& lhs, const struct ::passwd& rhs);

	template <class Ch>
	std::basic_istream<Ch>&
	operator>>(std::basic_istream<Ch>& in, basic_entity<Ch>& rhs);

	template <class Ch>
	std::basic_ostream<Ch>&
	operator<<(std::basic_ostream<Ch>& out, const basic_entity<Ch>& rhs);

	template <class Ch>
	sys::basic_bstream<Ch>&
	operator>>(sys::basic_bstream<Ch>& in, basic_entity<Ch>& rhs);

	template <class Ch>
	sys::basic_bstream<Ch>&
	operator<<(sys::basic_bstream<Ch>& out, const basic_entity<Ch>& rhs);

	void
	operator>>(const sqlite::statement& in, basic_entity<char>& rhs);

	typedef basic_entity<char> entity;

	template <class Ch>
	struct eq_traits<basic_entity<Ch>> {

		typedef basic_entity<Ch> entity_type;

		inline static bool
		eq(const entity_type& lhs, const entity_type& rhs) {
			return lhs.id() == rhs.id() || lhs.name() == rhs.name();
		}

	};

}

namespace std {

	template<class Ch>
	struct hash<ggg::basic_entity<Ch>>: public hash<basic_string<Ch>> {

		typedef size_t result_type;
		typedef ggg::basic_entity<Ch> argument_type;

		inline result_type
		operator()(const argument_type& rhs) const noexcept {
			return hash<basic_string<Ch>>::operator()(rhs.name());
		}

	};

}

#endif // ENTITY_HH
