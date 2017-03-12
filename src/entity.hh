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
		void
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
			char ch;
			if (in.get(ch) && ch != sep) {
				in.putback(ch);
			}
		}


		template<>
		void
		read_field<ignore_field>(std::istream& in, ignore_field&, char sep) {
			char ch;
			while (in.get(ch) and ch != sep and ch != '\n');
			if (in.eof()) {
				in.clear();
			}
		}

		template<>
		void
		read_field<char*>(std::istream& in, char*& rhs, char sep) {
			std::istream::sentry s(in);
			if (s) {
				if (rhs) {
					delete rhs;
					rhs = nullptr;
				}
				std::string tmp;
				char ch;
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
		}

		inline const char*
		non_null(const char* rhs) {
			return rhs ? rhs : "";
		}

	}

	class entity: public sys::user {

	public:
		entity() {
			std::memset(this, 0, sizeof(entity));
		}

		entity(const entity&) = delete;
		entity(entity&&) = delete;
		~entity() {
			delete this->pw_name;
			delete this->pw_passwd;
			#ifdef __linux__
			delete this->pw_gecos;
			#endif
			delete this->pw_dir;
			delete this->pw_shell;
		}

		friend std::istream&
		operator>>(std::istream& in, entity& rhs) {
			std::istream::sentry s(in);
			if (s) {
				bits::read_field(in, rhs.pw_name, ':');
				bits::read_field(in, rhs.pw_passwd, ':');
				bits::read_field(in, rhs.pw_uid, ':');
				bits::read_field(in, rhs.pw_gid, ':');
				#ifdef __linux__
				bits::read_field(in, rhs.pw_gecos, ':');
				#endif
				bits::read_field(in, rhs.pw_dir, ':');
				bits::read_field(in, rhs.pw_shell, ':');
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
