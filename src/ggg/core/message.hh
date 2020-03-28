#ifndef GGG_CORE_MESSAGE_HH
#define GGG_CORE_MESSAGE_HH

#include <chrono>
#include <ostream>
#include <string>

#include <sqlitex/statement.hh>

namespace ggg {

    class message {

    public:
        using clock_type = std::chrono::system_clock;
        using time_point = clock_type::time_point;
        using duration = clock_type::duration;

        static constexpr const char delimiter = ' ';

    private:
        std::string _name;
        std::string _hostname;
        std::string _text;
        time_point _timestamp;

    public:

        inline const std::string& text() const { return this->_text; }
        inline const std::string& hostname() const { return this->_hostname; }
        inline const std::string& name() const { return this->_name; }
        inline time_point timestamp() const { return this->_timestamp; }

        void clear();

        friend void operator>>(const sqlite::statement& in, message& rhs);
        friend std::ostream& operator<<(std::ostream& out, const message& rhs);

    };

    void operator>>(const sqlite::statement& in, message& rhs);
    std::ostream& operator<<(std::ostream& out, const message& rhs);

}

#endif // vim:filetype=cpp
