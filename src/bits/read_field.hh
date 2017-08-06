#ifndef READ_FIELD_HH
#define READ_FIELD_HH

#include <ostream>
#include <istream>
#include <iostream>

namespace ggg {

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
			std::ios::iostate old = in.exceptions();
			in.exceptions(std::ios::goodbit);
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
			in.exceptions(old);
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
			return ch;
		}

		template<class Ch, class Tr, class Alloc>
		inline char
		read_field(
			std::istream& in,
			std::basic_string<Ch,Tr,Alloc>& rhs,
			char sep
		) {
			char ch = 0;
			std::istream::sentry s(in);
			rhs.clear();
			if (s) {
				char prev = 0;
				while (in.get(ch) and
					(ch != sep || (ch == sep && prev == '\\')) and
					ch != '\n')
				{
					if (ch == sep && prev == '\\') {
						rhs.back() = sep;
					} else {
						rhs.push_back(ch);
					}
					prev = ch;
				}
				while (!rhs.empty() && std::isspace(rhs.back())) {
					rhs.pop_back();
				}
			}
			return ch;
		}


		inline void
		read_all_fields(std::istream&, char) {}

		template<class T, class ... Args>
		inline void
		read_all_fields(std::istream& in, char sep, T& first, Args& ... args) {
			// tolerate empty fields at the end of the line
			char ch = 0;
			while (in.get(ch) && std::isspace(ch) && ch != sep && ch != '\n');
			if (ch != sep && ch != '\n') {
				in.putback(ch);
				ch = read_field(in, first, sep);
			}
			if (ch == sep) {
				read_all_fields(in, sep, args...);
			}
		}

	}

}

#endif // READ_FIELD_HH
