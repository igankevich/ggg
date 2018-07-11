#include "native.hh"

#include <codecvt>
#include <iostream>

void
ggg
::init_locale() {
	try {
		init_locale(std::locale(""));
	} catch (const std::exception& err) {
		std::wcerr << "GGG: failed to init locale" << std::endl;
	}
}

void
ggg
::init_locale(std::locale rhs) {
	// enforce UTF-8 encoding
	{
		bool change = false;
		std::string name(rhs.name());
		auto idx = name.find('.');
		if (idx == std::string::npos) {
			change = true;
		} else {
			std::string suffix(name.substr(idx+1));
			for (char& ch : suffix) {
				ch = std::tolower(ch);
			}
			if (suffix != "utf8" && suffix != "utf-8") {
				change = true;
			}
		}
		if (change) {
			rhs = std::locale(name + ".UTF-8");
		}
	}
	std::locale::global(rhs);
	std::cout.imbue(std::locale::classic());
	std::wcout.imbue(std::locale::classic());
	std::cerr.imbue(std::locale::classic());
	std::clog.imbue(std::locale::classic());
	std::wcerr.imbue(std::locale::classic());
	std::wclog.imbue(std::locale::classic());
}
