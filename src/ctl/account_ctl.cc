#include "account_ctl.hh"

#include <fstream>
#include <algorithm>
#include <system_error>
#include <iostream>
#include <unistd.h>

#include "config.hh"
#include "bits/io.hh"

namespace {

	inline void
	rename_shadow() {
		ggg::bits::rename(GGG_SHADOW_NEW, GGG_SHADOW);
	}

}

ggg::account_ctl::iterator
ggg::account_ctl::find(const char* user) const {
	bits::check(GGG_SHADOW, R_OK);
	iterator result;
	std::ifstream shadow;
	try {
		shadow.exceptions(std::ios::badbit);
		shadow.open(GGG_SHADOW);
		result = std::find_if(
			iterator(shadow),
			iterator(),
			[user] (const account& rhs) {
				return rhs.login() == user;
			}
		);
	} catch (...) {
		bits::throw_io_error(shadow, "unable to read accounts from " GGG_SHADOW);
	}
	return result;
}

void
ggg::account_ctl::for_each(process_account func) {
	bits::check(GGG_SHADOW, R_OK);
	std::ifstream shadow;
	try {
		shadow.exceptions(std::ios::badbit);
		shadow.open(GGG_SHADOW);
		std::for_each(iterator(shadow), iterator(), func);
	} catch (...) {
		bits::throw_io_error(shadow, "unable to read accounts from " GGG_SHADOW);
	}
}

void
ggg::account_ctl::erase(const char* user) {
	bits::check(GGG_SHADOW, R_OK | W_OK);
	bits::check(GGG_SHADOW_NEW, R_OK | W_OK, false);
	std::ifstream shadow;
	std::ofstream shadow_new;
	try {
		shadow.exceptions(std::ios::badbit);
		shadow.open(GGG_SHADOW);
		shadow_new.exceptions(std::ios::badbit);
		shadow_new.open(GGG_SHADOW_NEW);
		std::copy_if(
			iterator(shadow),
			iterator(),
			oiterator(shadow_new, "\n"),
			[user,this] (const account& rhs) {
				bool match = rhs.login() == user;
				if (match && this->_verbose) {
					std::clog
						<< "removing " << user
						<< " from " << GGG_SHADOW
						<< std::endl;
				}
				return !match;
			}
		);
	} catch (...) {
		bits::throw_io_error(shadow, "unable to write accounts to " GGG_SHADOW_NEW);
	}
	rename_shadow();
}

void
ggg::account_ctl::update(const account& acc) {
	bits::check(GGG_SHADOW, R_OK | W_OK);
	bits::check(GGG_SHADOW_NEW, R_OK | W_OK, false);
	std::ifstream shadow;
	std::ofstream shadow_new;
	try {
		shadow.exceptions(std::ios::badbit);
		shadow.open(GGG_SHADOW);
		shadow_new.exceptions(std::ios::badbit);
		shadow_new.open(GGG_SHADOW_NEW);
		std::transform(
			iterator(shadow),
			iterator(),
			oiterator(shadow_new, "\n"),
			[&acc] (const account& rhs) {
				return rhs.login() == acc.login() ? acc : rhs;
			}
		);
	} catch (...) {
		bits::throw_io_error(shadow, "unable to write accounts to " GGG_SHADOW_NEW);
	}
	rename_shadow();
}

void
ggg::account_ctl::update(const char* acc, update_account func) {
	bits::check(GGG_SHADOW, R_OK | W_OK);
	bits::check(GGG_SHADOW_NEW, R_OK | W_OK, false);
	std::ifstream shadow;
	std::ofstream shadow_new;
	try {
		shadow.exceptions(std::ios::badbit);
		shadow.open(GGG_SHADOW);
		shadow_new.exceptions(std::ios::badbit);
		shadow_new.open(GGG_SHADOW_NEW);
		std::transform(
			iterator(shadow),
			iterator(),
			oiterator(shadow_new, "\n"),
			[acc,&func] (const account& rhs) {
				if (rhs.login() == acc) {
					func(const_cast<account&>(rhs));
				}
				return rhs;
			}
		);
	} catch (...) {
		bits::throw_io_error(shadow, "unable to write accounts to " GGG_SHADOW_NEW);
	}
	rename_shadow();
}

