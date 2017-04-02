#ifndef READ_FIELD_HH
#define READ_FIELD_HH

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

	}

}

#endif // READ_FIELD_HH
