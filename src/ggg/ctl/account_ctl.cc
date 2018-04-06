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
#include <unistdx/fs/path>

#include <ggg/bits/io.hh>
#include <ggg/config.hh>
#include <ggg/core/iacctree.hh>

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

	template <class Function>
	inline void
	for_each_account_file(Function func) {
		ggg::iacctree tree(sys::path(GGG_ROOT, "acc"));
		std::for_each(
			ggg::iacctree_iterator<sys::pathentry>(tree),
			ggg::iacctree_iterator<sys::pathentry>(),
			func
		);
	}

	template <class Function>
	inline void
	find_in_account_files(Function func) {
		ggg::iacctree tree(sys::path(GGG_ROOT, "acc"));
		std::find_if(
			ggg::iacctree_iterator<sys::pathentry>(tree),
			ggg::iacctree_iterator<sys::pathentry>(),
			func
		);
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
		    std::clog << "updating " << new_acc.login() << std::endl;
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
		std::clog << "appending " << acc.login()
		          << " to " << dest << std::endl;
	}
	bits::append(acc, dest, "unable to add account", this->getperms(uid));
}

void
ggg::account_ctl
::open() {
	this->_accounts.clear();
	ggg::iacctree tree(sys::path(GGG_ROOT, "acc"));
	std::for_each(
		ggg::iacctree_iterator<sys::pathentry>(tree),
		ggg::iacctree_iterator<sys::pathentry>(),
		[this] (const sys::pathentry& entry) {
		    sys::path f(entry.getpath());
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
		        std::stringstream msg;
		        msg << "unable to read accounts from " << f;
		        bits::throw_io_error(in, msg.str());
			}
		}
	);
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
			std::clog << "deactivating " << tmp.login() << std::endl;
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
			std::clog << "activating " << tmp.login() << std::endl;
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
			std::clog << "resetting password for " << tmp.login() << std::endl;
		}
		tmp.reset_password();
		this->update(tmp);
	}
}

