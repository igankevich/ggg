#ifndef GGG_CORE_NATIVE_HH
#define GGG_CORE_NATIVE_HH

#include <locale>
#include <ostream>
#include <stdexcept>
#include <string>

#include <unistdx/base/check>

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

	inline const char*
	native_n(const char* id, unsigned long n) {
		return text_domain(GGG_CATALOG).text_n(id, id, n);
	}

	inline std::ostream&
	format_message(std::ostream& out, char, const char* s) {
		return out << s;
	}

	template<class T, class ... Args>
	inline std::ostream&
	format_message(
		std::ostream& out,
		char ch,
		const char* s,
		const T& value,
		const Args& ... args
	) {
		while (*s && *s != ch) {
			out << *s++;
		}
		out << value;
		if (!*s) { return out; }
		return format_message(out, ch, ++s, args ...);
	}

	template<class ... Args>
	inline std::ostream&
	format_message(std::ostream& out, const char* s, const Args& ... args) {
		return format_message(out, '_', s, args ...);
	}

	template<class ... Args>
	inline std::ostream&
	native_message(std::ostream& out, const char* s, const Args& ... args) {
		std::string str = native(s);
		str.push_back('\n');
		return format_message(out, str.data(), args ...);
	}

	template<class ... Args>
	inline std::ostream&
	native_message_n(
		std::ostream& out,
		unsigned long n,
		const char* s,
		const Args& ... args
	) {
		std::string str = native_n(s, n);
		str.push_back('\n');
		return format_message(out, str.data(), args ...);
	}

	template<class ... Args>
	inline std::ostream&
	native_sentence(std::ostream& out, const char* s, const Args& ... args) {
		return format_message(out, native(s), args ...);
	}

	inline std::ostream&
	error_sentence(std::ostream& out, const std::exception& err) {
		return out << native(err.what());
	}

	inline std::ostream&
	error_message(std::ostream& out, const std::exception& err) {
		return error_sentence(out, err) << std::endl;
	}

}

#endif // vim:filetype=cpp
