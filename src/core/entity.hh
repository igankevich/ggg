#ifndef ENTITY_HH
#define ENTITY_HH

#include <functional>
#include <istream>
#include <string>

#include <unistdx/fs/path>
#include <unistdx/util/user>

#include "form_field.hh"

namespace ggg {

	class form_field;

	class entity {

	public:
		typedef const size_t columns_type[7];
		static constexpr const char delimiter = ':';

	private:
		std::string _name;
		std::string _password;
		std::string _realname;
		std::string _homedir;
		std::string _shell;
		sys::uid_type _uid = -1;
		sys::gid_type _gid = -1;
		sys::path _origin;

	public:

		inline explicit
		entity(const char* name):
		_name(name)
		{}

		inline explicit
		entity(const char* name, sys::uid_type uid, sys::gid_type gid):
		_name(name),
		_homedir("/home/"),
		_shell("/bin/sh"),
		_uid(uid),
		_gid(gid)
		{ this->_homedir.append(_name); }

		inline explicit
		entity(const ::passwd& rhs):
		_name(rhs.pw_name),
		_password(rhs.pw_passwd),
		_realname(rhs.pw_gecos),
		_homedir(rhs.pw_dir),
		_shell(rhs.pw_shell),
		_uid(rhs.pw_uid),
		_gid(rhs.pw_gid)
		{}

		entity() = default;
		entity(const entity& rhs) = default;
		entity(entity&& rhs) = default;
		~entity() = default;

		entity&
		operator=(const entity& rhs) = default;

		void
		copy_to(struct ::passwd* lhs, char* buffer) const;

		size_t
		buffer_size() const noexcept;

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
		operator==(const entity& rhs) const noexcept {
			return name() == rhs.name();
		}

		inline bool
		operator!=(const entity& rhs) const noexcept {
			return !operator==(rhs);
		}

		inline bool
		operator<(const entity& rhs) const noexcept {
			return name() < rhs.name();
		}

		friend std::istream&
		operator>>(std::istream& in, entity& rhs);

		friend std::ostream&
		operator<<(std::ostream& out, const entity& rhs);

		void
		set(const form_field& field, const char* value);

		void
		write_human(std::ostream& out, columns_type width) const;

		std::istream&
		read_human(std::istream& in);

		inline sys::uid_type
		id() const noexcept {
			return _uid;
		}

		inline sys::gid_type
		gid() const noexcept {
			return _gid;
		}

		inline const std::string&
		name() const noexcept {
			return _name;
		}

		bool
		has_valid_name() const noexcept;

		inline const std::string&
		password() const noexcept {
			return _password;
		}

		inline const std::string&
		real_name() const noexcept {
			return _realname;
		}

		inline const std::string&
		home() const noexcept {
			return _homedir;
		}

		inline const std::string&
		shell() const noexcept {
			return _shell;
		}

		inline const sys::path&
		origin() const noexcept {
			return this->_origin;
		}

		inline void
		origin(const sys::path& rhs) {
			this->_origin = rhs;
		}

		inline bool
		has_origin() const noexcept {
			return !this->_origin.empty();
		}

		void clear();

		void
		merge(const entity& rhs);
	};


	std::istream&
	operator>>(std::istream& in, entity& rhs);

	std::ostream&
	operator<<(std::ostream& out, const entity& rhs);

}

namespace std {

	template<>
	struct hash<ggg::entity> {

		typedef size_t result_type;
		typedef ggg::entity argument_type;

		size_t
		operator()(const ggg::entity& rhs) const noexcept {
			return std::hash<std::string>()(rhs.name());
		}

	};

}

#endif // ENTITY_HH
