#include "account_ctl.hh"

#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <system_error>

#include <unistdx/base/check>
#include <unistdx/fs/canonical_path>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/path>

#include <ggg/bits/io.hh>
#include <ggg/config.hh>
#include <ggg/core/native.hh>

namespace {

	inline void
	rename_shadow() {
		ggg::bits::rename(GGG_SHADOW_NEW, GGG_SHADOW);
	}

	inline sys::path
	tmp_file(sys::path rhs) {
		sys::path p(rhs);
		p += ".new";
		return p;
	}

}

void
ggg::account_ctl
::erase(const char* user) {
	const_iterator result = this->find(user);
	if (result != this->end()) {
		const sys::uid_type uid = sys::this_process::user();
		bits::erase<account,char_type>(
			*result,
			this->_verbose,
			this->getperms(uid)
		);
	}
}

void
ggg::account_ctl
::update(const account& acc) {
	if (acc.origin().empty()) {
		throw std::invalid_argument("bad origin");
	}
	const_iterator result = this->_accounts.find(acc);
	if (result == this->end()) {
		throw std::invalid_argument("bad account");
	}
	account old(*result);
	this->_accounts.erase(result);
	this->_accounts.insert(acc);
	try {
		// update without touching the password
		account new_acc(old);
		new_acc.copy_from(acc);
		if (this->verbose()) {
			native_message(std::clog, "Updating _.", new_acc.login());
		}
		const sys::uid_type uid = sys::this_process::user();
		bits::update<account,char_type>(
			new_acc,
			this->_verbose,
			this->getperms(uid)
		);
	} catch (...) {
		this->_accounts.erase(acc);
		this->_accounts.insert(old);
		throw;
	}
}

void
ggg::account_ctl
::update_password(const account& acc) {
	if (acc.origin().empty()) {
		throw std::invalid_argument("bad origin");
	}
	const sys::uid_type uid = sys::this_process::user();
	bits::update<account,char_type>(acc, this->_verbose, this->getperms(uid));
}

void
ggg::account_ctl
::add(const account& acc) {
	if (acc.login().empty()) {
		throw std::invalid_argument("bad login");
	}
	const_iterator result = this->_accounts.find(acc);
	if (result != this->end()) {
		throw std::invalid_argument("bad account");
	}
	const sys::uid_type uid = sys::this_process::user();
	sys::path dest;
	if (uid == 0) {
		dest = GGG_SHADOW;
	} else if (!acc.origin().empty()) {
		dest = acc.origin();
	} else {
		throw std::invalid_argument("bad origin");
	}
	if (this->verbose()) {
		native_message(std::clog, "Appeding _ to _.", acc.login(), dest);
	}
	bits::append(acc, dest, "unable to add account", this->getperms(uid));
}

void
ggg::account_ctl
::open() {
	this->_accounts.clear();
	sys::idirtree tree(sys::path(GGG_ROOT, "acc"));
	while (!tree.eof()) {
		tree.clear();
		try {
			sys::directory_entry entry;
			while (tree >> entry) {
				if (sys::get_file_type(tree.current_dir(), entry) != sys::file_type::regular) {
					continue;
				}
				sys::path f(tree.current_dir(), entry.name());
				std::ifstream in;
				try {
					in.imbue(std::locale::classic());
					in.exceptions(std::ios::badbit);
					in.open(f);
					std::for_each(
						std::istream_iterator<account>(in),
						std::istream_iterator<account>(),
						[this,&f] (const account& rhs) {
							account acc(rhs);
							acc.origin(f);
							this->_accounts.insert(acc);
						}
					);
				} catch (...) {
					// skip unreadable files
				}
			}
		} catch (...) {
			// skip unreadable directories
		}
	}
}

void
ggg::account_ctl
::expire(const char* user) {
	const_iterator result = this->find(user);
	if (result == this->end()) {
		throw std::invalid_argument("bad account");
	}
	if (!result->has_expired()) {
		account tmp(*result);
		if (this->verbose()) {
			native_message(std::clog, "Deactivating _.", tmp.login());
		}
		tmp.make_expired();
		this->update(tmp);
	}
}

void
ggg::account_ctl
::activate(const char* user) {
	const_iterator result = this->find(user);
	if (result == this->end()) {
		throw std::invalid_argument("bad account");
	}
	if (result->has_expired()) {
		account tmp(*result);
		if (this->verbose()) {
			native_message(std::clog, "Activating _.", tmp.login());
		}
		tmp.make_active();
		this->update(tmp);
	}
}

void
ggg::account_ctl
::expire_password(const char* user) {
	const_iterator result = this->find(user);
	if (result == this->end()) {
		throw std::invalid_argument("bad account");
	}
	if (!result->password_has_expired()) {
		account tmp(*result);
		if (this->verbose()) {
			native_message(std::clog, "Resetting password for _.", tmp.login());
		}
		tmp.reset_password();
		this->update(tmp);
	}
}

