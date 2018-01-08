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
	std::locale::global(std::locale(""));
	std::cout.imbue(std::locale::classic());
	std::wcout.imbue(std::locale::classic());
	std::cerr.imbue(std::locale::classic());
	std::clog.imbue(std::locale::classic());
	std::wcerr.imbue(std::locale::classic());
	std::wclog.imbue(std::locale::classic());
}

