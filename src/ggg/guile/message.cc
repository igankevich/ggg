#include <chrono>
#include <ostream>
#include <locale>
#include <ctime>
#include <iomanip>
#include <locale>
#include <ratio>
#include <sstream>
#include <istream>

#include <ggg/core/message.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/store.hh>

namespace ggg {

    template <>
    void
    Guile_traits<ggg::message>::to_guile(std::ostream& guile, const array_type& objects) {
        const char format[] = "%Y-%m-%dT%H:%M:%S%z";
        if (objects.empty()) { guile << "(list)"; return; }
        std::string indent(2, ' ');
        guile << "(list";
        for (const auto& msg : objects) {
            char s[128] {};
            auto t = message::clock_type::to_time_t(msg.timestamp());
            std::strftime(s, sizeof(s), format, std::localtime(&t));
            guile << '\n' << indent;
            guile << "(make <message>\n";
            guile << indent << "      #:name " << escape_string(msg.name()) << '\n';
            guile << indent << "      #:hostname " << escape_string(msg.hostname()) << '\n';
            guile << indent << "      #:timestamp (time-point "
                << escape_string(s) << ")\n";
            guile << indent << "      #:text " << escape_string(msg.text()) << ')';
        }
        guile << ")\n";
    }

}

