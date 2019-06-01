#ifndef GGG_CLI_GUILE_TRAITS_HH
#define GGG_CLI_GUILE_TRAITS_HH

#include <libguile.h>

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
		static std::string to_guile(const T& x);
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

	std::string
	escape_string(const std::string& s);

}

#endif // vim:filetype=cpp
