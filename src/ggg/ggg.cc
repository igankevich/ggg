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

void
ggg::Ggg::update(const entity& ent) {
	this->_hierarchy.update(ent);
}

void
ggg::Ggg::update(const account& acc) {
	// update without touching the password
	this->_accounts.update(
		acc.login().data(),
		[&acc,this] (account& existing) {
			existing.copy_from(acc);
			if (this->verbose()) {
				std::clog << "updating " << existing.login() << std::endl;
			}
		}
	);
}

