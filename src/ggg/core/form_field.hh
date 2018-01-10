#ifndef CORE_FORM_FIELD_HH
#define CORE_FORM_FIELD_HH

#include <string>
#include <ostream>
#include <istream>
#include <functional>
#include <ggg/sec/secure_string.hh>

namespace ggg {

	enum struct field_type: int {
		text,
		password,
		constant,
		set,
		set_secure,
	};

	std::istream&
	operator>>(std::istream& in, field_type& rhs);

	std::ostream&
	operator<<(std::ostream& out, field_type rhs);

	field_type
	to_field_type(const std::string& s);

	class form_field {

	public:
		typedef unsigned int id_type;

		static constexpr const char delimiter = ':';

	private:
		id_type _id = 0;
		std::string _name;
		std::string _regex;
		std::string _target;
		std::string _value;
		secure_string _secvalue;
		field_type _type = field_type::text;

	public:
		form_field() = default;
		form_field(const form_field&) = default;
		form_field(form_field&&) = default;
		form_field& operator=(const form_field&) = default;
		~form_field() = default;

		inline explicit
		form_field(id_type id):
		_id(id)
		{}

		inline id_type
		id() const noexcept {
			return this->_id;
		}

		inline const std::string&
		name() const noexcept {
			return this->_name;
		}

		inline const std::string&
		regex() const noexcept {
			return this->_regex;
		}

		inline const std::string&
		target() const noexcept {
			return this->_target;
		}

		inline const std::string&
		value() const noexcept {
			return this->_value;
		}

		inline const secure_string&
		secure_value() const noexcept {
			return this->_secvalue;
		}

		inline bool
		is_constant() const noexcept {
			return this->_type == field_type::constant;
		}

		inline bool
		is_input() const noexcept {
			return this->_type == field_type::text ||
				this->_type == field_type::password;
		}

		inline field_type
		type() const noexcept {
			return this->_type;
		}

		void clear();

		inline bool
		operator==(const form_field& rhs) const noexcept {
			return this->_id == rhs._id;
		}

		inline bool
		operator!=(const form_field& rhs) const noexcept {
			return !operator==(rhs);
		}

		inline bool
		operator<(const form_field& rhs) const noexcept {
			return this->_id < rhs._id;
		}

		friend std::istream&
		operator>>(std::istream& in, form_field& rhs);

		friend std::ostream&
		operator<<(std::ostream& out, const form_field& rhs);

		friend void
		swap(form_field& lhs, form_field& rhs);

	};

	std::istream&
	operator>>(std::istream& in, form_field& rhs);

	std::ostream&
	operator<<(std::ostream& out, const form_field& rhs);

	void swap(form_field& lhs, form_field& rhs);

}

namespace std {

	template<>
	struct hash<ggg::form_field> {

		typedef size_t result_type;
		typedef ggg::form_field argument_type;

		size_t
		operator()(const ggg::form_field& rhs) const noexcept {
			return std::hash<ggg::form_field::id_type>()(rhs.id());
		}

	};

}

#endif // CORE_FORM_FIELD_HH
