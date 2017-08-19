#ifndef ENTITY_HH
#define ENTITY_HH

#include <istream>
#include <string>
#include <functional>
#include <unistdx/fs/path>
#include <unistdx/util/user>

namespace ggg {

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
