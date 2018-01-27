#ifndef GGG_NATIVE_HH
#define GGG_NATIVE_HH

#include <string>
#include <ostream>

namespace ggg {

	/// Set default system locale, but retain classic
	/// locale for output and error streams.
	void
	init_locale();

	std::string
	native(const char* text);

	template <class Ch>
	std::basic_ostream<Ch>&
	format_message(std::basic_ostream<Ch>& out, char, const char* s) {
		return out << s;
	}

	template<class Ch, class T, class ... Args>
	std::basic_ostream<Ch>&
	format_message(
		std::basic_ostream<Ch>& out,
		char ch,
		const char* s,
		const T& value,
		const Args& ... args
	) {
		while (*s && *s != ch) {
			out << *s++;
		}
		out << value;
		return format_message(out, ch, ++s, args ...);
	}

	template<class Ch, class ... Args>
	std::basic_ostream<Ch>&
	format_message(
		std::basic_ostream<Ch>& out,
		const char* s,
		const Args& ... args
	) {
		return format_message(out, '_', s, args ...);
	}

	template<class Ch, class ... Args>
	std::basic_ostream<Ch>&
	native_message(
		std::basic_ostream<Ch>& out,
		const char* s,
		const Args& ... args
	) {
		std::string str = native(s);
		return format_message(out, str.data(), args...);
	}

}

#endif // GGG_NATIVE_HH