#ifndef GGG_CLI_GUILE_TRAITS_HH
#define GGG_CLI_GUILE_TRAITS_HH

#include <libguile.h>

#include <iosfwd>
#include <string>
#include <vector>

#include <ggg/core/guile_traits.hh>

namespace ggg {

	template <class T>
	struct Guile_traits {
		using array_type = std::vector<T>;
		using string_array = std::vector<std::string>;
		static T from(SCM x);
		static SCM to(const T& x);
		static SCM insert(SCM args);
		static SCM remove(SCM args);
		static SCM remove_all();
		static SCM find();
		static array_type select(std::string name);
		static void define_procedures();
		static void to_guile(std::ostream& out, const array_type& list);
		static array_type generate(const string_array& names);

		inline static array_type
		from_guile(std::string guile) {
			array_type result;
			auto list = scm_c_eval_string(guile.data());
			if (scm_to_bool(scm_list_p(list))) {
				auto n = scm_to_int32(scm_length(list));
				for (int i=0; i<n; ++i) {
					result.emplace_back(from(scm_list_ref(list, scm_from_int32(i))));
				}
			} else {
				result.emplace_back(from(list));
			}
			return result;
		}

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

	inline SCM
	slot(SCM x, SCM name) {
		return scm_slot_ref(x, name);
	}

	inline bool
	slot_is_bound(SCM x, SCM name) {
		return scm_to_bool(scm_slot_bound_p(x, name));
	}

	inline bool is_string(SCM x) { return scm_to_bool(scm_string_p(x)); }

	std::string escape_string(const std::string& s);
	std::string file_to_string(std::string filename);

}

#endif // vim:filetype=cpp
