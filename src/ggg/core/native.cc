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
//	typedef std::codecvt_utf8<wchar_t> codecvt_facet;
//	if (!std::has_facet<codecvt_facet>(rhs)) {
//		rhs = std::locale(rhs, new codecvt_facet);
//	}
	std::locale::global(rhs);
	std::cout.imbue(std::locale::classic());
	std::wcout.imbue(std::locale::classic());
	std::cerr.imbue(std::locale::classic());
	std::clog.imbue(std::locale::classic());
	std::wcerr.imbue(std::locale::classic());
	std::wclog.imbue(std::locale::classic());
}
