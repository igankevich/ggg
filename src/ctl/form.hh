#ifndef CTL_FORM_HH
#define CTL_FORM_HH

#include <core/form_field.hh>
#include <core/account.hh>
#include <core/entity.hh>
#include <pam/pam_handle.hh>
#include <vector>
#include <unordered_map>

namespace ggg {

	enum struct form_type {
		console,
		graphical
	};

	class form {

	public:
		typedef std::vector<form_field> container_type;

	private:
		container_type all_fields;
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

		void
		read_fields(const account& recruiter);

		entity
		input_entity(ggg::pam_handle* pamh);

	private:

		bool
		validate(const responses& r);

		entity
		make_entity(const responses& r);

	};

	typedef std::unordered_map<ggg::form_field,const char*> field_values;

	std::string
	interpolate(std::string orig, const field_values& values, char prefix='$');

}

#endif // CTL_FORM_HH vim:filetype=cpp
