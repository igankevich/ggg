#ifndef ENTITY_HH
#define ENTITY_HH

#include <istream>
#include <string>
#include <cstring>
#include <fstream>
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
		read_field<char*>(std::istream& in, char*& rhs, char sep) {
			std::istream::sentry s(in);
			char ch = 0;
			if (s) {
				if (rhs) {
					delete rhs;
					rhs = nullptr;
				}
				std::string tmp;
				while (in.get(ch) and ch != sep and ch != '\n') {
					tmp.push_back(ch);
				}
				while (!tmp.empty() && std::isspace(tmp.back())) {
					tmp.pop_back();
				}
				std::string::size_type n = tmp.size();
				rhs = new char[n + 1];
				std::strcpy(rhs, tmp.data());
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
	}

	class entity: public sys::user {

	public:

		explicit
		entity(const char* name) {
			std::memset(this, 0, sizeof(entity));
			this->pw_name = bits::create_and_copy(name);
			init_ids();
		}

		entity() {
			std::memset(this, 0, sizeof(entity));
			init_ids();
		}

		entity(const entity& rhs) {
			this->pw_name = bits::create_and_copy(rhs.pw_name);
			this->pw_passwd = bits::create_and_copy(rhs.pw_passwd);
			#ifdef __linux__
			this->pw_gecos = bits::create_and_copy(rhs.pw_gecos);
			#endif
			this->pw_dir = bits::create_and_copy(rhs.pw_dir);
			this->pw_shell = bits::create_and_copy(rhs.pw_shell);
			this->pw_uid = rhs.pw_uid;
			this->pw_gid = rhs.pw_gid;
		}

		entity(entity&& rhs) {
			this->pw_name = move_pointer(&rhs.pw_name);
			this->pw_passwd = move_pointer(&rhs.pw_passwd);
			#ifdef __linux__
			this->pw_gecos = move_pointer(&rhs.pw_gecos);
			#endif
			this->pw_dir = move_pointer(&rhs.pw_dir);
			this->pw_shell = move_pointer(&rhs.pw_shell);
			this->pw_uid = rhs.pw_uid;
			this->pw_gid = rhs.pw_gid;
		}

		~entity() {
			delete this->pw_name;
			delete this->pw_passwd;
			#ifdef __linux__
			delete this->pw_gecos;
			#endif
			delete this->pw_dir;
			delete this->pw_shell;
		}

		entity&
		operator=(const entity& rhs) {
			this->pw_name = bits::create_and_copy(rhs.pw_name);
			this->pw_passwd = bits::create_and_copy(rhs.pw_passwd);
			#ifdef __linux__
			this->pw_gecos = bits::create_and_copy(rhs.pw_gecos);
			#endif
			this->pw_dir = bits::create_and_copy(rhs.pw_dir);
			this->pw_shell = bits::create_and_copy(rhs.pw_shell);
			this->pw_uid = rhs.pw_uid;
			this->pw_gid = rhs.pw_gid;
			return *this;
		}

		void
		copy_to(struct ::passwd& lhs) const {
			lhs.pw_name = bits::create_and_copy(this->pw_name);
			lhs.pw_passwd = bits::create_and_copy(this->pw_passwd);
			#ifdef __linux__
			lhs.pw_gecos = bits::create_and_copy(this->pw_gecos);
			#endif
			lhs.pw_dir = bits::create_and_copy(this->pw_dir);
			lhs.pw_shell = bits::create_and_copy(this->pw_shell);
			lhs.pw_uid = this->pw_uid;
			lhs.pw_gid = this->pw_gid;
		}

		inline bool
		has_id() const noexcept {
			return this->pw_uid != sys::uid_type(-1);
		}

		inline bool
		operator==(const entity& rhs) const noexcept {
			return (name() == nullptr && rhs.name() == nullptr)
				|| (std::strcmp(name(), rhs.name()) == 0);
		}

		inline bool
		operator!=(const entity& rhs) const noexcept {
			return !operator==(rhs);
		}

		inline bool
		operator<(const entity& rhs) const noexcept {
			return name() && rhs.name() && std::strcmp(name(), rhs.name()) < 0;
		}

		friend std::istream&
		operator>>(std::istream& in, entity& rhs);

		friend std::ostream&
		operator<<(std::ostream& out, const entity& rhs);

	private:

		char*
		move_pointer(char** rhs) {
			char* ptr;
			if (*rhs == nullptr) {
				ptr = nullptr;
			} else {
				ptr = *rhs;
				*rhs = nullptr;
			}
			return ptr;
		}

		void
		init_ids() {
			this->pw_uid = -1;
			this->pw_gid = -1;
			this->pw_shell = bits::create_and_copy("/bin/sh");
			this->pw_dir = bits::create_and_copy("/");
		}

	};


	std::istream&
	operator>>(std::istream& in, entity& rhs);

	std::ostream&
	operator<<(std::ostream& out, const entity& rhs);

}

#endif // ENTITY_HH
