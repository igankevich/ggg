#include "ggg.hh"

#include <iostream>

#include "config.hh"

void
ggg::Ggg::erase(const std::string& user) {
	this->_hierarchy.erase(user.data());
	this->_accounts.erase(user.data());
}

void
ggg::Ggg::expire(const std::string& user) {
	this->_accounts.update(
		user.data(),
		[&] (account& rhs) {
			if (!rhs.has_expired()) {
				if (this->verbose()) {
					std::clog << "deactivating " << rhs.login() << std::endl;
				}
				rhs.make_expired();
			}
		}
	);
}

void
ggg::Ggg::activate(const std::string& user) {
	this->_accounts.update(
		user.data(),
		[&] (account& rhs) {
			if (rhs.has_expired()) {
				if (this->verbose()) {
					std::clog << "activating " << rhs.login() << std::endl;
				}
				rhs.make_active();
			}
		}
	);
}
