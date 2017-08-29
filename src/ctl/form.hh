#ifndef CTL_FORM_HH
#define CTL_FORM_HH

#include <core/form_field.hh>
#include <core/form_type.hh>
#include <core/account.hh>
#include <core/entity.hh>
#include <pam/pam_handle.hh>
#include <vector>
#include <tuple>
#include <unordered_map>

namespace ggg {

	class form {

	public:
		typedef std::vector<form_field> container_type;

	private:
		container_type _fields;
		form_type _type = form_type::console;

	public:

		form() = default;
		form(const form&) = default;
		form(form&&) = default;
		form& operator=(const form&) = default;

		inline explicit
		form(const account& recruiter) {
			this->read_fields(recruiter);
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
		read_fields(const account& recruiter);

		std::tuple<entity,account>
		input_entity(ggg::pam_handle* pamh);

	private:

		bool
		validate(const responses& r);

		std::tuple<entity,account>
		make_entity_and_account(const responses& r, pam_handle* pamh);

	};

	typedef std::unordered_map<ggg::form_field,const char*> field_values;

	std::string
	interpolate(std::string orig, const field_values& values, char prefix='$');

}

#endif // CTL_FORM_HH vim:filetype=cpp
