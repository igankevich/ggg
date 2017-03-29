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
		char
		read_field<ignore_field>(std::istream& in, ignore_field&, char sep) {
			char ch = 0;
			while (in.get(ch) and ch != sep and ch != '\n');
			if (in.eof()) {
				in.clear();
			}
			return ch;
		}

		template<>
		char
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

		inline const char*
		non_null(const char* rhs) {
			return rhs ? rhs : "";
		}

	}

	class entity: public sys::user {

	public:

		entity(const char* name) {
			std::memset(this, 0, sizeof(entity));
			this->pw_name = create_and_copy(name);
			init_ids();
		}

		entity() {
			std::memset(this, 0, sizeof(entity));
			init_ids();
		}

		entity(const entity& rhs) {
			this->pw_name = create_and_copy(rhs.pw_name);
			this->pw_passwd = create_and_copy(rhs.pw_passwd);
			#ifdef __linux__
			this->pw_gecos = create_and_copy(rhs.pw_gecos);
			#endif
			this->pw_dir = create_and_copy(rhs.pw_dir);
			this->pw_shell = create_and_copy(rhs.pw_shell);
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

		bool
		has_id() const noexcept {
			return this->pw_uid != sys::uid_type(-1);
		}

		bool
		operator==(const entity& rhs) const noexcept {
			return (name() == nullptr && rhs.name() == nullptr)
				|| (std::strcmp(name(), rhs.name()) == 0);
		}

		bool
		operator!=(const entity& rhs) const noexcept {
			return !operator==(rhs);
		}

		bool
		operator<(const entity& rhs) const noexcept {
			return name() && rhs.name() && std::strcmp(name(), rhs.name()) < 0;
		}

		friend std::istream&
		operator>>(std::istream& in, entity& rhs) {
			std::istream::sentry s(in);
			if (s) {
				char sep = bits::read_field(in, rhs.pw_name, ':');
				if (sep == ':') {
					bits::read_field(in, rhs.pw_passwd, ':');
					bits::read_field(in, rhs.pw_uid, ':');
					bits::read_field(in, rhs.pw_gid, ':');
					#ifdef __linux__
					bits::read_field(in, rhs.pw_gecos, ':');
					#endif
					bits::read_field(in, rhs.pw_dir, ':');
					bits::read_field(in, rhs.pw_shell, ':');
				}
			}
			return in;
		}

		friend std::ostream&
		operator<<(std::ostream& out, const entity& rhs) {
			using bits::non_null;
			return out
				<< non_null(rhs.name()) << ':'
				<< non_null(rhs.password()) << ':'
				<< rhs.id() << ':'
				<< rhs.group_id() << ':'
				<< non_null(rhs.real_name()) << ':'
				<< non_null(rhs.home()) << ':'
				<< non_null(rhs.shell());
		}

	private:
		char*
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
		}

	};

	class entity_database: private sys::dirtree {

		typedef sys::dirtree tree_type;
		typedef sys::pathentry entry_type;
		enum struct dbstate {
			scanning,
			reading
		};

		entry_type _entry;
		dbstate _dbstate = dbstate::scanning;
		std::ifstream _fstr;

		void
		setdbstate(dbstate rhs) noexcept {
			_dbstate = rhs;
		}

		bool
		is_scanning() const noexcept {
			return _dbstate == dbstate::scanning;
		}

		bool
		is_reading() const noexcept {
			return _dbstate == dbstate::reading;
		}

	public:

		entity_database&
		operator>>(entity& rhs) {
			while (is_scanning() && !eof()) {
				if (sys::dirtree::operator>>(_entry)) {
					_fstr.open(_entry.getpath());
					setdbstate(dbstate::reading);
				} else if (!eof()) {
					clear();
				}
			}
			if (is_reading()) {
				if (_fstr) {
					_fstr >> rhs;
				} else {
					_fstr.close();
					setdbstate(dbstate::scanning);
				}
			}
			return *this;
		}
	};

}

#endif // ENTITY_HH
