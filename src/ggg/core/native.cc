#include "native.hh"

#include <iostream>

namespace {

	class Messages {

	public:
		typedef std::messages<char> facet_type;
		typedef std::messages_base::catalog catalog_type;

	private:
		std::locale _locale;
		catalog_type _catalog = -1;

	public:
		Messages() {
			try {
				this->open(std::locale(""));
			} catch (const std::exception& err) {
				this->error("open");
				try {
					this->open(std::locale::classic());
				} catch (const std::exception& err) {
					this->error("open");
					throw;
				}
			}
		}

		~Messages() {
			try {
				this->facet().close(this->_catalog);
			} catch (const std::exception& err) {
				this->error("close");
			}
		}

		void
		open(std::locale rhs) {
			this->_locale = rhs;
			this->_catalog = this->facet().open(GGG_CATALOG, this->_locale);
			if (this->_catalog < 0) {
				throw std::runtime_error("failed to open message catalog");
			}
		}

		void
		error(const char* func) {
			std::cerr << "GGG: failed to " << func << " message catalog for "
			          << this->_locale.name() << " locale" << std::endl;
		}

		std::string
		get(const char* text) const {
			return this->facet().get(this->_catalog, 0, 0, text);
		}

		const facet_type&
		facet() const {
			return std::use_facet<facet_type>(this->_locale);
		}

	};

	Messages messages;

}

std::string
ggg
::native(const char* text) {
	return messages.get(text);
}

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
