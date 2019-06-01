#ifndef GGG_CLI_GUILE_TRAITS_HH
#define GGG_CLI_GUILE_TRAITS_HH

#include <libguile.h>

#include <string>
#include <vector>

#include <ggg/core/guile_traits.hh>

namespace ggg {

	template <class T>
	struct Guile_traits {
		using array_type = std::vector<T>;
		static T from(SCM x);
		static SCM to(const T& x);
		static SCM insert(SCM args);
		static SCM remove(SCM args);
		static SCM remove_all();
		static SCM find();
		static void define_procedures();
		static std::string to_guile(const T& x, size_t shift=0, bool shift_first=false);
		static array_type from_guile(std::string guile);
	};

	inline std::string
	to_string(SCM s) {
		char* tmp = scm_to_utf8_string(s);
		std::string ret(tmp);
		std::free(tmp);
		return ret;
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
