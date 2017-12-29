#ifndef READ_FIELD_HH
#define READ_FIELD_HH

#include <ostream>
#include <istream>
#include <iostream>

namespace ggg {

	namespace bits {

		struct ignore_field {

			template <class Ch>
			inline friend std::basic_ostream<Ch>&
			operator<<(std::basic_ostream<Ch>& out, const ignore_field&) {
				return out;
			}

		};

		template<class Ch, class T>
		char
		read_field(std::basic_istream<Ch>& in, T& rhs, Ch sep) {
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
			Ch ch = 0;
			if (in.get(ch) && ch != sep) {
				in.putback(ch);
			}
			return ch;
		}

		template<class Ch>
		inline Ch
		read_field(std::basic_istream<Ch>& in, ignore_field&, Ch sep) {
			Ch ch = 0;
			while (in.get(ch) and ch != sep and ch != '\n');
			return ch;
		}

		template<class Ch>
		inline Ch
		read_field(
			std::basic_istream<Ch>& in,
			std::basic_string<Ch>& rhs,
			Ch sep
		) {
			Ch ch = 0;
			typename std::basic_istream<Ch>::sentry s(in);
			rhs.clear();
			if (s) {
				Ch prev = 0;
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

		template <class Ch>
		inline void
		read_all_fields(std::basic_istream<Ch>&, Ch) {}

		template<class Ch, class T, class ... Args>
		inline void
		read_all_fields(
			std::basic_istream<Ch>& in,
			Ch sep,
			T& first,
			Args& ... args
		) {
			// tolerate empty fields at the end of the line
			Ch ch = 0;
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
