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
#include <unistdx/fs/file_mode>
#include <unistdx/fs/path>
#include <unistdx/ipc/identity>

#include <ggg/bits/io.hh>
#include <ggg/config.hh>
#include <ggg/core/iacctree.hh>

namespace {

	inline void
	rename_shadow() {
		ggg::bits::rename(GGG_SHADOW_NEW, GGG_SHADOW);
	}

	inline void
	set_mode(const char* filename, sys::file_mode m) {
		UNISTDX_CHECK(::chmod(filename, m));
	}

	inline void
	set_owner(const char* filename, sys::uid_type uid, sys::gid_type gid) {
		UNISTDX_CHECK(::chown(filename, uid, gid));
	}

	inline void
	set_perms(
		const char* filename,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m
	) {
		set_owner(filename, uid, gid);
		set_mode(filename, m);
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
	if (result == this->end()) {
		return;
	}
	bits::erase<account,char_type>(*result, this->_verbose);
}

void
ggg::account_ctl
::update(const account& acc) {
	if (acc.origin().empty()) {
		throw std::invalid_argument("bad origin");
	}
	const sys::uid_type uid = sys::this_process::user();
	const sys::gid_type gid = sys::this_process::group();
	const sys::file_mode m = uid == 0 ? 0 : 0600;
	bits::update<account,char_type>(acc, this->_verbose);
}

void
ggg::account_ctl
::add(const account& acc) {
	if (acc.login().empty()) {
		throw std::invalid_argument("bad login");
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
	bits::append(acc, dest, "unable to add account");
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
