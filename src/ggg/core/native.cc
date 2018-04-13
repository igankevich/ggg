#include "native.hh"

#include <iostream>

void
ggg
::init_locale() {
	try {
		init_locale(std::locale(""));
	} catch (const std::exception& err) {
		std::cerr << "GGG: failed to init locale" << std::endl;
	}
}

void
ggg
::init_locale(std::locale rhs) {
	std::locale::global(rhs);
	std::cout.imbue(std::locale::classic());
	std::wcout.imbue(std::locale::classic());
	std::cerr.imbue(std::locale::classic());
	std::clog.imbue(std::locale::classic());
	std::wcerr.imbue(std::locale::classic());
	std::wclog.imbue(std::locale::classic());
}
