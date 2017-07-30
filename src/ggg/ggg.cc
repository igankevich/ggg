#include "ggg.hh"

#include "config.hh"

void
ggg::Ggg::erase(const char* user) {
	if (!user) {
		throw std::invalid_argument("bad entity");
	}
	this->_hierarchy.erase(user);
	this->_accounts.erase(user);
}
