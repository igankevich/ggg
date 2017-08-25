#include "ggg.hh"

#include <iostream>
#include <system_error>
#include <unistd.h>

#include "config.hh"

void
ggg::GGG::erase(const std::string& user) {
	this->_hierarchy.erase(user.data());
	this->_accounts.erase(user.data());
}

void
ggg::GGG::expire(const std::string& user) {
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
ggg::GGG::activate(const std::string& user) {
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

bool
ggg::GGG::contains(const std::string& name) {
	return this->_hierarchy.find_by_name(name.data()) != this->_hierarchy.end()
		|| this->_accounts.find(name.data()) != this->_accounts.end();
}

void
ggg::GGG::update(const entity& ent) {
	this->_hierarchy.update(ent);
}

void
ggg::GGG::update(const account& acc) {
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

ggg::entity
ggg::GGG::generate(const std::string& name) {
	return this->_hierarchy.generate(name.data());
}

void
ggg::GGG::add(const entity& ent, const std::string& filename) {
	this->add(ent, filename, this->_accounts.generate(ent.name().data()));
}

void
ggg::GGG::add(
	const entity& ent,
	const std::string& filename,
	const account& acc
) {
	this->mkdirs(this->to_relative_path(filename));
	this->_hierarchy.add(ent, filename);
	this->_accounts.add(acc);
}

std::string
ggg::GGG::to_relative_path(const std::string& filename) {
	bool is_absolute = !filename.empty() && filename[0] == '/';
	std::string relative_path;
	if (is_absolute) {
		sys::canonical_path realpath(filename);
		if (!realpath.is_relative_to(this->_hierarchy.root())) {
			throw std::invalid_argument(
				"file path must be relative to hierarchy root"
			);
		}
		const size_t n = this->_hierarchy.root().size();
		relative_path = realpath.substr(0, n+1);
	} else {
		relative_path = filename;
	}
	return relative_path;
}

void
ggg::GGG::mkdirs(std::string relative_path) {
	// split path into components ignoring the last one
	// since it is a file
	size_t i0 = 0, i1 = 0;
	while ((i1 = relative_path.find('/', i0)) != std::string::npos) {
		sys::path p(this->_hierarchy.root(), relative_path.substr(0, i1));
		sys::file_stat st(p);
		if (!st.exists()) {
			if (this->verbose()) {
				std::clog << "making " << p << std::endl;
			}
			if (-1 == ::mkdir(p, 0755)) {
				throw std::system_error(errno, std::system_category());
			}
		}
		i0 = i1 + 1;
	}
}

