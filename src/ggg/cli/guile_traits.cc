#include <cctype>
#include <sstream>

#include <ggg/cli/guile_traits.hh>

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
			default: if (std::isprint(ch)) { result += ch; }
					 else {
						 std::stringstream tmp;
						 tmp << std::hex << int(ch);
						 result += tmp.str();
					 }
					 break;
		}
	}
	result += '"';
	return result;
}

