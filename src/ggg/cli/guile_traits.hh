#ifndef GGG_CLI_GUILE_TRAITS_HH
#define GGG_CLI_GUILE_TRAITS_HH

#include <libguile.h>

#include <string>

#include <ggg/core/guile_traits.hh>

namespace ggg {

	template <class T>
	struct Guile_traits {
		static const char* name();
		static SCM insert(SCM args);
		static SCM remove(SCM args);
		static SCM remove_all();
		static void define_procedures();
	};

	inline std::string
	to_string(SCM s) {
		char* tmp = scm_to_utf8_string(s);
		scm_dynwind_free(tmp);
		return tmp;
	}

	inline bool is_bound(SCM s) { return !SCM_UNBNDP(s); }

}

#endif // vim:filetype=cpp
