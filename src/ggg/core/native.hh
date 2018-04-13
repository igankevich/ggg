#ifndef GGG_NATIVE_HH
#define GGG_NATIVE_HH

#include <locale>
#include <ostream>
#include <stdexcept>
#include <string>

#include <unistdx/base/check>

#include <ggg/bits/to_bytes.hh>
#include <ggg/config.hh>

namespace ggg {

	class text_domain {

	public:
		typedef unsigned long int number_type;

	private:
		const char* _domain = "donotexist";

	public:

		text_domain() = delete;

		inline explicit
		text_domain(const char* domain):
		_domain(domain)
		{}

		inline const char*
		directory() const {
			return ::bindtextdomain(this->_domain, nullptr);
		}

		inline const char*
		directory(const char* rhs) const {
			const char* ret = ::bindtextdomain(this->_domain, rhs);
			if (!ret && rhs) {
				UNISTDX_THROW_BAD_CALL();
			}
			return ret;
		}

		inline const char*
		codeset() const {
			return ::bind_textdomain_codeset(this->_domain, nullptr);
		}

		inline const char*
		codeset(const char* rhs) const {
			const char* ret = ::bind_textdomain_codeset(this->_domain, rhs);
			if (!ret && rhs) {
				UNISTDX_THROW_BAD_CALL();
			}
			return ret;
		}

		inline const char*
		text(const char* id) const noexcept {
			return ::dgettext(this->_domain, id);
		}

		inline const char*
		text(const char* id, int category) const noexcept {
			return ::dcgettext(this->_domain, id, category);
		}

		inline const char*
		text_n(
			const char* id,
			const char* id_plural,
			number_type n
		) const noexcept {
			return ::dngettext(this->_domain, id, id_plural, n);
		}

		inline const char*
		text_n(
			const char* id,
			const char* id_plural,
			number_type n,
			int category
		) const noexcept {
			return ::dcngettext(this->_domain, id, id_plural, n, category);
		}

		inline friend std::ostream&
		operator<<(std::ostream& out, const text_domain& rhs) {
			const char* cs = rhs.codeset();
			return out << "directory=" << rhs.directory()
			           << ",codeset=" << (cs ? cs : "<default>");
		}

		inline static const char*
		global(const char* domain) {
			const char* ret = ::textdomain(domain);
			if (ret == nullptr) {
				UNISTDX_THROW_BAD_CALL();
			}
			return ret;
		}

	};

	/// Set default system locale, but retain classic
	/// locale for output and error streams.
	void
	init_locale();

	void
	init_locale(std::locale rhs);

	inline const char*
	native(const char* id) {
		return text_domain(GGG_CATALOG).text(id);
	}

	inline std::wstring
	wnative(const char* id, bits::wcvt_type& cv) {
		return cv.from_bytes(native(id));
	}

	inline const char*
	native_n(const char* id, unsigned long n) {
		return text_domain(GGG_CATALOG).text_n(id, id, n);
	}

	template <class Ch>
	std::basic_ostream<Ch>&
	format_message(std::basic_ostream<Ch>& out, Ch, const Ch* s) {
		return out << s;
	}

	template<class Ch, class T, class ... Args>
	std::basic_ostream<Ch>&
	format_message(
		std::basic_ostream<Ch>& out,
		Ch ch,
		const Ch* s,
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
		const Ch* s,
		const Args& ... args
	) {
		return format_message<Ch>(out, Ch('_'), s, args ...);
	}

	template<class Ch, class ... Args>
	std::basic_ostream<Ch>&
	native_message(
		std::basic_ostream<Ch>& out,
		const char* s,
		const Args& ... args
	) {
		bits::wcvt_type cv;
		std::basic_string<Ch> str = bits::to_bytes<Ch>(cv, native(s));
		str.push_back('\n');
		return format_message(out, str.data(), args ...);
	}

	template<class Ch, class ... Args>
	std::basic_ostream<Ch>&
	native_message_n(
		std::basic_ostream<Ch>& out,
		unsigned long n,
		const char* s,
		const Args& ... args
	) {
		bits::wcvt_type cv;
		std::basic_string<Ch> str = bits::to_bytes<Ch>(cv, native_n(s, n));
		str.push_back('\n');
		return format_message(out, str.data(), args ...);
	}

	inline std::ostream&
	error_message(std::ostream& out, const std::exception& err) {
		return out << native(err.what()) << std::endl;
	}

	inline std::wostream&
	error_message(std::wostream& out, const std::exception& err) {
		bits::wcvt_type cv;
		return out << cv.from_bytes(native(err.what())) << std::endl;
	}

}

#endif // GGG_NATIVE_HH
