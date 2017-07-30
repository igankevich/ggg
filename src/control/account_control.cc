#include "account_control.hh"

#include <fstream>
#include <algorithm>
#include <system_error>

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

}

ggg::Account_control::iterator
ggg::Account_control::find(const char* user) const {
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
ggg::Account_control::erase(const char* user) {
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
				return rhs.login() != user;
			}
		);
	} catch (...) {
		throw_io_error(shadow, "unable to write accounts to " GGG_SHADOW_NEW);
	}
	rename_shadow();
}

void
ggg::Account_control::update(const account& acc) {
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
