#include "native.hh"

#include <locale>
#include <iostream>
#include "config.hh"

namespace {

	class Messages {

	public:
		typedef std::messages<char> facet_type;
		typedef std::messages_base::catalog catalog_type;

	private:
		std::locale _locale;
		const facet_type& _facet;
		catalog_type _catalog;

	public:
		Messages():
		_locale(""),
		_facet(std::use_facet<facet_type>(_locale)),
		_catalog(_facet.open(GGG_CATALOG, _locale))
		{}

		~Messages() {
			this->_facet.close(this->_catalog);
		}

		std::string
		get(const char* text) const {
			return this->_facet.get(this->_catalog, 0, 0, text);
		}

	};

	Messages messages;

}

std::string
ggg::native(const char* text) {
	return messages.get(text);
}

void
ggg::init_locale() {
	/*
	std::locale loc("");
	std::cerr.imbue(loc);
	std::clog.imbue(loc);
	std::cout.imbue(loc);
	std::wcerr.imbue(loc);
	std::wclog.imbue(loc);
	std::wcout.imbue(loc);
	*/
}

