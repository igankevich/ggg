#ifndef GGG_NATIVE_HH
#define GGG_NATIVE_HH

#include <string>
#include <ostream>

namespace ggg {

	void
	init_locale();

	std::string
	native(const char* text);

	inline std::ostream&
	format_message(std::ostream& out, const char* s) {
		return out << s;
	}

	template<class T, class ... Args>
	std::ostream&
	format_message(
		std::ostream& out,
		const char* s,
		const T& value,
		const Args& ... args
	) {
		while (*s != '_') {
			out << *s++;
		}
		out << value;
		return format_message(out, ++s, args ...);
	}

	template<class T, class ... Args>
	std::ostream&
	native_message(
		std::ostream& out,
		const char* s,
		const T& value,
		const Args& ... args
	) {
		std::string str = native(s);
		return format_message(out, str.data(), value, args...);
	}

}

#endif // GGG_NATIVE_HH
