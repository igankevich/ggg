#ifndef GGG_CLI_GUILE_TRAITS_HH
#define GGG_CLI_GUILE_TRAITS_HH

#include <libguile.h>

#include <iosfwd>
#include <string>
#include <vector>

#include <ggg/core/guile_traits.hh>

namespace ggg {

	inline void
	guile_throw(const std::exception& err) {
		scm_throw(
			scm_from_utf8_symbol("ggg-error"),
			scm_list_1(scm_from_utf8_string(err.what()))
		);
	}

	template <class T>
	struct Guile_traits {

		using array_type = std::vector<T>;
		using string_array = std::vector<std::string>;

		static T from(SCM x);
		static SCM to(const T& x);
		static SCM insert0(SCM args);
		static SCM remove(SCM args);
		static SCM remove_all();
		static SCM find();
		static array_type select(std::string name);
		static void define_procedures();
		static void to_guile(std::ostream& out, const array_type& list);
		static array_type generate(const string_array& names);

		static inline SCM
		insert(SCM args) {
			try {
				return insert0(args);
			} catch (const std::exception& err) {
				guile_throw(err);
				return SCM_UNSPECIFIED;
			}
		}

		static inline array_type
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

	template <class Pointer>
	inline void
	define_procedure(const char *name, int req, int opt, int rest, Pointer ptr) {
		scm_c_define_gsubr(name, req, opt, rest, reinterpret_cast<scm_t_subr>(ptr));
		scm_c_export(name);
	}

	inline SCM
	from_pointer(void* ptr) {
		return scm_from_intptr_t(reinterpret_cast<intptr_t>(ptr));
	}

}

#endif // vim:filetype=cpp
