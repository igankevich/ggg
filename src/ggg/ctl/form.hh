#ifndef CTL_FORM_HH
#define CTL_FORM_HH

#include <locale>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <ggg/core/account.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/form_field.hh>
#include <ggg/core/form_type.hh>

namespace ggg {

	class form {

	public:
		typedef std::vector<form_field> container_type;

	private:
		container_type _fields;
		form_type _type = form_type::console;
		double _minentropy = 30.0;
		std::locale _locale = std::locale::classic();

	public:

		form() = default;

		form(const form&) = default;

		form(form&&) = default;

		form&
		operator=(const form&) = default;

		inline explicit
		form(const char* name) {
			this->open(name);
		}

		inline void
		set_type(form_type rhs) noexcept {
			this->_type = rhs;
		}

		inline bool
		is_console() const noexcept {
			return this->_type == form_type::console;
		}

		inline const container_type&
		fields() const noexcept {
			return this->_fields;
		}

		void
		open(const char* name);

		inline void
		clear() {
			this->_fields.clear();
		}

		inline double
		min_entropy() const noexcept {
			return this->_minentropy;
		}

		inline const std::locale&
		locale() const noexcept {
			return this->_locale;
		}

		std::tuple<entity,account>
		make_entity_and_account(std::vector<std::string> responses);

	};

	typedef std::unordered_map<ggg::form_field,const char*> field_values;

	std::string
	interpolate(std::string orig, const field_values& values, char prefix='$');

}

#endif // CTL_FORM_HH vim:filetype=cpp
