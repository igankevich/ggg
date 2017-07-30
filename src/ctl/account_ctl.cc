#include "account_ctl.hh"

#include <fstream>
#include <algorithm>
#include <system_error>
#include <iostream>
#include <unistd.h>

#include "config.hh"

namespace {

	inline void
	throw_io_error(std::istream& in, const char* msg) {
		if (!in.eof()) {
			throw std::system_error(std::io_errc::stream, msg);
		}
	}

	inline void
	rename_shadow() {
		int ret = std::rename(GGG_SHADOW_NEW, GGG_SHADOW);
		if (ret == -1) {
			throw std::system_error(errno, std::system_category());
		}
	}

	inline void
	check(const char* path, int mode, bool must_exist=true) {
		if (-1 == ::access(path, mode)) {
			if ((errno == ENOENT && must_exist) || errno != ENOENT) {
				throw std::system_error(errno, std::system_category());
			}
		}
	}

}

ggg::account_ctl::iterator
ggg::account_ctl::find(const char* user) const {
	check(GGG_SHADOW, R_OK);
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
		throw_io_error(shadow, "unable to read accounts from " GGG_SHADOW);
	}
	return result;
}

void
ggg::account_ctl::erase(const char* user) {
	check(GGG_SHADOW, R_OK | W_OK);
	check(GGG_SHADOW_NEW, R_OK | W_OK, false);
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
			[user] (const account& rhs) {
				bool match = rhs.login() == user;
				if (match) {
					std::clog
						<< "removing " << user
						<< " from " << GGG_SHADOW
						<< std::endl;
				}
				return !match;
			}
		);
	} catch (...) {
		throw_io_error(shadow, "unable to write accounts to " GGG_SHADOW_NEW);
	}
	rename_shadow();
}

void
ggg::account_ctl::update(const account& acc) {
	check(GGG_SHADOW, R_OK | W_OK);
	check(GGG_SHADOW_NEW, R_OK | W_OK);
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
		throw_io_error(shadow, "unable to write accounts to " GGG_SHADOW_NEW);
	}
	rename_shadow();
}
