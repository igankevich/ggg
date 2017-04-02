#ifndef STRING_REPLACE_HH
#define STRING_REPLACE_HH

#include <cstdlib>

void
string_replace(
	std::string& str,
	const std::string& olds,
	const std::string& news
) {
	std::string::size_type pos = 0u;
	while((pos = str.find(olds, pos)) != std::string::npos){
		str.replace(pos, olds.length(), news);
		pos += news.length();
	}
}

int
run_script(std::string script, std::string arg) {
	string_replace(script, "%", arg);
	return std::system(script.data());
}

#endif // STRING_REPLACE_HH
