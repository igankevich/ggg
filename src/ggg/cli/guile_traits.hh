#ifndef GGG_CLI_GUILE_TRAITS_HH
#define GGG_CLI_GUILE_TRAITS_HH

#include <libguile.h>

#include <cctype>
#include <string>

#include <ggg/core/guile_traits.hh>

namespace ggg {

	template <class T>
	struct Guile_traits {
		static T from(SCM x);
		static SCM to(const T& x);
		static SCM insert(SCM args);
		static SCM remove(SCM args);
		static SCM remove_all();
		static SCM find();
		static void define_procedures();
	};

	inline std::string
	to_string(SCM s) {
		char* tmp = scm_to_utf8_string(s);
		scm_dynwind_free(tmp);
		return tmp;
	}

	inline bool is_bound(SCM s) { return !SCM_UNBNDP(s); }

	inline SCM
	slot(SCM x, const char* name) {
		return scm_slot_ref(x, scm_from_latin1_symbol(name));
	}

	inline std::string
	s_expression_string(const std::string& s) {
		std::string result;
		for (auto ch : s) {
			switch (ch) {
				case '\\': result += R"(\\)"; break;
				case '"': result += R"(\")"; break;
				case 7: result += R"(\a)"; break;
				case 12: result += R"(\f)"; break;
				case 10: result += R"(\n)"; break;
				case 13: result += R"(\r)"; break;
				case 9: result += R"(\t)"; break;
				case 11: result += R"(\v)"; break;
				case 8: result += R"(\b)"; break;
				case 0: result += R"(\0)"; break;
				default: if (std::isprint(ch)) { result += ch; } else { /* TODO */ } break;
			}
		}
		return result;
	}

}

#endif // vim:filetype=cpp
