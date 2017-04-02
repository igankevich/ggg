#ifndef ENTITY_HH
#define ENTITY_HH

#include <istream>
#include <string>
#include <sys/dir.hh>
#include <sys/users.hh>

namespace legion {

	namespace bits {

		struct ignore_field {

			inline friend std::ostream&
			operator<<(std::ostream& out, const ignore_field&) {
				return out;
			}

		};

		template<class T>
		char
		read_field(std::istream& in, T& rhs, char sep) {
			in >> rhs;
			// tolerate empty and erroneous CSV fields
			if (in.rdstate() & std::ios::failbit) {
				rhs = T();
				in.clear();
				ignore_field tmp;
				read_field(in, tmp, sep);
			}
			if (in.rdstate() & (std::ios::eofbit | std::ios::failbit)) {
				in.clear();
			}
			in >> std::ws;
			char ch = 0;
			if (in.get(ch) && ch != sep) {
				in.putback(ch);
			}
			return ch;
		}


		template<>
		inline char
		read_field<ignore_field>(std::istream& in, ignore_field&, char sep) {
			char ch = 0;
			while (in.get(ch) and ch != sep and ch != '\n');
			if (in.eof()) {
				in.clear();
			}
			return ch;
		}

		template<>
		inline char
		read_field<std::string>(std::istream& in, std::string& rhs, char sep) {
			char ch = 0;
			std::istream::sentry s(in);
			if (s) {
				rhs.clear();
				while (in.get(ch) and ch != sep and ch != '\n') {
					rhs.push_back(ch);
				}
				while (!rhs.empty() && std::isspace(rhs.back())) {
					rhs.pop_back();
				}
				if (in.eof()) {
					in.clear();
				}
			}
			return ch;
		}


		inline void
		read_all_fields(std::istream&, char) {}

		template<class T, class ... Args>
		inline void
		read_all_fields(std::istream& in, char sep, T& first, Args& ... args) {
			char lastchar = read_field(in, first, sep);
			if (lastchar == sep) {
				read_all_fields(in, sep, args...);
			}
		}

		inline char*
		create_and_copy(const char* rhs) {
			char* ptr;
			if (rhs == nullptr) {
				ptr = nullptr;
			} else {
				const size_t n = std::strlen(rhs);
				ptr = new char[n];
				std::strcpy(ptr, rhs);
			}
			return ptr;
		}

		inline char*
		bufcopy(char** field, char* dest, const char* src) {
			*field = dest;
			while ((*dest++ = *src++));
			return dest;
		}
	}

	class entity {

		std::string _username;
		std::string _password;
		std::string _realname;
		std::string _homedir = "/";
		std::string _shell = "/bin/sh";
		sys::uid_type _uid = -1;
		sys::gid_type _gid = -1;

	public:

		explicit
		entity(const char* name):
		_username(name)
		{}

		entity() = default;
		entity(const entity& rhs) = default;
		entity(entity&& rhs) = default;
		~entity() = default;

		entity&
		operator=(const entity& rhs) = default;

		void
		copy_to(struct ::passwd* lhs, char* buffer) const {
			buffer = bits::bufcopy(&lhs->pw_name, buffer, this->_username.data());
			buffer = bits::bufcopy(&lhs->pw_passwd, buffer, this->_password.data());
			#ifdef __linux__
			buffer = bits::bufcopy(&lhs->pw_gecos, buffer, this->_realname.data());
			#endif
			buffer = bits::bufcopy(&lhs->pw_dir, buffer, this->_homedir.data());
			buffer = bits::bufcopy(&lhs->pw_shell, buffer, this->_shell.data());
			lhs->pw_uid = this->_uid;
			lhs->pw_gid = this->_gid;
		}

		size_t
		buffer_size() const noexcept {
			return this->_username.size()
				+ this->_password.size()
				+ this->_realname.size()
				+ this->_homedir.size()
				+ this->_shell.size();
		}

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

		sys::uid_type
		group_id() const noexcept {
			return _gid;
		}

		const std::string&
		name() const noexcept {
			return _username;
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

	};


	std::istream&
	operator>>(std::istream& in, entity& rhs);

	std::ostream&
	operator<<(std::ostream& out, const entity& rhs);

}

#endif // ENTITY_HH
