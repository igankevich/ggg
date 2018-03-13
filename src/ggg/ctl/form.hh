#ifndef CTL_FORM_HH
#define CTL_FORM_HH

#include <tuple>
#include <unordered_map>
#include <vector>

#include <ggg/core/account.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/form_field.hh>
#include <ggg/core/form_type.hh>
#ifndef GGG_DISABLE_PAM
#include <ggg/pam/pam_handle.hh>
#endif

namespace ggg {

	class form {

	public:
		typedef std::vector<form_field> container_type;

	private:
		container_type _fields;
		form_type _type = form_type::console;
		double _minentropy = 0.0;

	public:

		form() = default;

		form(const form&) = default;

		form(form&&) = default;

		form&
		operator=(const form&) = default;

		inline explicit
		form(const char* name, double minentropy):
		_minentropy(minentropy) {
			this->read_fields(name);
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
		read_fields(const char* name);

		#ifndef GGG_DISABLE_PAM
		std::tuple<entity,account>
		input_entity(ggg::pam_handle* pamh);
		#endif

		std::tuple<entity,account>
		input_entity();

	private:

		#ifndef GGG_DISABLE_PAM
		std::vector<bool>
		validate(const responses& r);

		std::tuple<entity,account>
		make_entity_and_account(const responses& r, pam_handle* pamh);
		#endif

	};

	typedef std::unordered_map<ggg::form_field,const char*> field_values;

	std::string
	interpolate(std::string orig, const field_values& values, char prefix='$');

}

#endif // CTL_FORM_HH vim:filetype=cpp
