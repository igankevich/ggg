#ifndef GGG_CORE_PUBLIC_KEY_HH
#define GGG_CORE_PUBLIC_KEY_HH

#include <ostream>
#include <string>

#include <sqlitex/statement.hh>

#include <ggg/core/guile_traits.hh>

namespace ggg {

    class public_key {

    private:
        std::string _name;
        std::string _options;
        std::string _type;
        std::string _key;
        std::string _comment;

    public:
        inline explicit public_key(std::string name): _name(name) {}
        public_key() = default;
        ~public_key() = default;
        public_key(const public_key&) = default;
        public_key& operator=(const public_key&) = default;
        public_key(public_key&&) = default;
        public_key& operator=(public_key&&) = default;
        inline const std::string& name() const { return this->_name; }
        inline const std::string& options() const { return this->_options; }
        inline const std::string& type() const { return this->_type; }
        inline const std::string& key() const { return this->_key; }
        inline const std::string& comment() const { return this->_comment; }
        inline void name(const std::string& rhs) { this->_name = rhs; }
        inline void options(const std::string& rhs) { this->_options = rhs; }
        inline void type(const std::string& rhs) { this->_type = rhs; }
        inline void key(const std::string& rhs) { this->_key = rhs; }
        inline void comment(const std::string& rhs) { this->_comment = rhs; }
        void clear();
		friend void operator>>(const sqlite::statement& in, public_key& rhs);
		friend std::ostream& operator<<(std::ostream& out, const public_key& rhs);
		friend struct Guile_traits<public_key>;
    };

    void operator>>(const sqlite::statement& in, public_key& rhs);
    std::ostream& operator<<(std::ostream& out, const public_key& rhs);

    inline bool operator==(const public_key& a, const public_key& b) {
        return a.name() == b.name() && a.key() == b.key();
    }

}

#endif // vim:filetype=cpp
