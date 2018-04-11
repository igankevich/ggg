#ifndef GGG_NATIVE_HH
#define GGG_NATIVE_HH

#include <locale>
#include <ostream>
#include <string>

#include <ggg/config.hh>

namespace ggg {

	/// Set default system locale, but retain classic
	/// locale for output and error streams.
	void
	init_locale();

	void
	init_locale(std::locale rhs);

	std::string
	native(const char* text);

	inline std::string
	native_n(const char* text, unsigned long n) {
		return ::dngettext(GGG_CATALOG, text, text, n);
	}

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
		str.push_back('\n');
		return format_message(out, str.data(), args...);
	}

	template<class Ch, class ... Args>
	std::basic_ostream<Ch>&
	native_message_n(
		std::basic_ostream<Ch>& out,
		unsigned long n,
		const char* s,
		const Args& ... args
	) {
		std::string str = native_n(s, n);
		str.push_back('\n');
		return format_message(out, str.data(), args...);
	}

}

#endif // GGG_NATIVE_HH
