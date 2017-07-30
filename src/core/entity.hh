#ifndef ENTITY_HH
#define ENTITY_HH

#include <istream>
#include <string>
#include <sys/dir.hh>
#include <sys/path.hh>
#include <sys/users.hh>

namespace ggg {

	class entity {

		std::string _name;
		std::string _password;
		std::string _realname;
		std::string _homedir = "/";
		std::string _shell = "/bin/sh";
		sys::uid_type _uid = -1;
		sys::gid_type _gid = -1;
		sys::path _origin;

	public:

		explicit
		entity(const char* name):
		_name(name)
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

		sys::uid_type
		id() const noexcept {
			return _uid;
		}

		sys::gid_type
		gid() const noexcept {
			return _gid;
		}

		const std::string&
		name() const noexcept {
			return _name;
		}

		const std::string&
		password() const noexcept {
			return _password;
		}

		const std::string&
		real_name() const noexcept {
			return _realname;
		}

		const std::string&
		home() const noexcept {
			return _homedir;
		}

		const std::string&
		shell() const noexcept {
			return _shell;
		}

		const sys::path&
		origin() const noexcept {
			return this->_origin;
		}

		inline void
		origin(const sys::path& rhs) {
			this->_origin = rhs;
		}

		inline bool
		has_origin() const noexcept {
			return !this->_origin.to_string().empty();
		}

	};


	std::istream&
	operator>>(std::istream& in, entity& rhs);

	std::ostream&
	operator<<(std::ostream& out, const entity& rhs);

}

#endif // ENTITY_HH
