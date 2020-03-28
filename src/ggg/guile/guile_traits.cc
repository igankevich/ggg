#include <cctype>
#include <fstream>
#include <sstream>

#include <ggg/core/native.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/store.hh>

std::string
ggg::escape_string(const std::string& s) {
    std::string result;
    result += '"';
    for (auto ch : s) {
        switch (ch) {
            case '\\': result += R"(\\)"; break;
            case '"': result += R"(\")"; break;
            case 0: result += R"(\0)"; break;
            case 7: result += R"(\a)"; break;
            case 8: result += R"(\b)"; break;
            case 9: result += R"(\t)"; break;
            case 10: result += R"(\n)"; break;
            case 11: result += R"(\v)"; break;
            case 12: result += R"(\f)"; break;
            case 13: result += R"(\r)"; break;
            default: result += ch;
        }
    }
    result += '"';
    return result;
}


std::string
ggg::file_to_string(std::string filename) {
    std::ifstream in;
    std::stringstream guile;
    try {
        in.exceptions(std::ios::failbit | std::ios::badbit);
        in.imbue(std::locale::classic());
        in.open(filename, std::ios_base::in);
        guile << in.rdbuf();
        in.close();
    } catch (...) {
        if (!in.eof()) {
            throw std::system_error(std::io_errc::stream,
                native("Unable to read entities."));
        }
    }
    return guile.str();
}
