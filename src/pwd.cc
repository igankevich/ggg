#include "pwd.hh"

#include <fstream>
#include <string>
#include <cstring>
#include <sys/dir.hh>
#include <sys/users.hh>

namespace {

	typedef sys::dirtree_iterator<sys::pathentry> iterator;
	sys::dirtree root;
	iterator first;
	iterator last;
	std::ifstream in;

	namespace bits {

		struct ignore_field {

			friend std::ostream&
			operator<<(std::ostream& out, const ignore_field&) {
				return out;
			}

		};

		template<class T>
		void
		read_field(std::istream& in, T& rhs, char sep=':') {
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
		}

		template<>
		void
		read_field<ignore_field>(std::istream& in, ignore_field&, char sep) {
			char ch;
			while (in.get(ch) and ch != sep and ch != '\n');
			if (in) in.putback(ch);
		}

		template<>
		void
		read_field<char*>(std::istream& in, char*& rhs, char sep) {
			std::istream::sentry s(in);
			if (s) {
				if (rhs) {
					delete rhs;
				}
				std::string tmp;
				char ch;
				while (in.get(ch) and ch != sep and ch != '\n') {
					tmp.push_back(ch);
				}
				if (ch == sep) in.putback(ch);
				std::string::size_type n = tmp.size();
				rhs = new char[n + 1];
				std::strcpy(rhs, tmp.data());
			}
		}

	}

	class pwentry: public sys::user {

		pwentry() = default;
		pwentry(const pwentry&) = delete;
		pwentry(pwentry&&) = delete;
		~pwentry() {
			delete this->pw_name;
			delete this->pw_passwd;
			#ifdef __linux__
			delete this->pw_gecos;
			#endif
			delete this->pw_dir;
			delete this->pw_shell;
		}

		friend std::istream&
		operator>>(std::istream& in, pwentry& rhs) {
			std::istream::sentry s(in);
			if (s) {
				bits::read_field(in, rhs.pw_name);
				bits::read_field(in, rhs.pw_passwd);
				bits::read_field(in, rhs.pw_uid);
				bits::read_field(in, rhs.pw_gid);
				#ifdef __linux__
				bits::read_field(in, rhs.pw_gecos);
				#endif
				bits::read_field(in, rhs.pw_dir);
				bits::read_field(in, rhs.pw_shell);
			}
			return in;
		}
	};

}

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, pw) {
	root.clear();
	root.open(sys::path(legion::root_directory));
	enum nss_status ret;
	if (!root.is_open()) {
		ret = NSS_STATUS_UNAVAIL;
	} else {
		first = iterator(root);
		ret = NSS_STATUS_SUCCESS;
	}
	return ret;
}

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, pw) {
	root.close();
	first = iterator();
	in.close();
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, pw)(
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, uid)(
	::uid_t uid,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, nam)(
	const char* name,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}
