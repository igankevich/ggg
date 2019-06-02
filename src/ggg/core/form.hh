#ifndef GGG_CORE_FORM_HH
#define GGG_CORE_FORM_HH

#include <iosfwd>
#include <string>

#include <sqlitex/statement.hh>

#include <ggg/core/guile_traits.hh>

namespace ggg {

	class form2 {

	private:
		std::string _name;
		std::string _content;

	public:
		form2() = default;
		inline explicit form2(const char* name): _name(name) {}
		inline const std::string& name() const { return this->_name; }
		inline const std::string& content() const { return this->_content; }
		inline void clear() { this->_name.clear(); this->_content.clear(); }
		inline bool operator<(const form2& rhs) const { return this->_name < rhs._name; }
		inline bool operator==(const form2& rhs) const { return this->_name == rhs._name; }
		friend void operator>>(const sqlite::statement& in, form2& rhs);
		friend struct Guile_traits<form2>;

	};

	void operator>>(const sqlite::statement& in, form2& rhs);
	std::ostream& operator<<(std::ostream& out, const form2& rhs);

}

template <>
struct std::hash<ggg::form2>: public std::hash<std::string> {

	typedef size_t result_type;
	typedef ggg::form2 argument_type;

	inline result_type
	operator()(const argument_type& rhs) const {
		return this->hash<std::string>::operator()(rhs.name());
	}

};

#endif // vim:filetype=cpp
