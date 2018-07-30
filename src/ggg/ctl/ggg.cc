#include "ggg.hh"

#include <iostream>
#include <system_error>
#include <unistd.h>

#include <unistdx/fs/path>

#include <ggg/bits/to_bytes.hh>
#include <ggg/config.hh>
#include <ggg/core/native.hh>

namespace {

	const char* default_home = "/home";
	const char* default_shell = "/bin/sh";

}

template <class Ch>
void
ggg::basic_ggg<Ch>
::erase(const string_type& user) {
	this->_hierarchy.erase(user.data());
	this->_accounts.erase(user.data());
}

template <class Ch>
bool
ggg::basic_ggg<Ch>
::contains(const string_type& name) {
	return this->_hierarchy.find_by_name(name.data()) != this->_hierarchy.end()
	       || this->_accounts.find(name.data()) != this->_accounts.end();
}

template <class Ch>
typename ggg::basic_ggg<Ch>::entity_type
ggg::basic_ggg<Ch>
::generate(const string_type& name) {
	typedef sys::basic_path<Ch> path_type;
	entity_type result = this->_hierarchy.generate(name.data());
	result.home(path_type(default_home, result.name()));
	result.shell(default_shell);
	return result;
}

template <class Ch>
std::set<typename ggg::basic_ggg<Ch>::entity_type>
ggg::basic_ggg<Ch>
::generate(const std::unordered_set<string_type>& names) {
	typedef sys::basic_path<Ch> path_type;
	std::set<entity_type> result;
	this->_hierarchy.generate(
		names.begin(),
		names.end(),
		std::inserter(result, result.begin())
	);
	for (const entity_type& cent : result) {
		entity_type& ent = const_cast<entity_type&>(cent);
		ent.home(path_type(default_home, ent.name()));
		ent.shell(default_shell);
	}
	return result;
}

template <class Ch>
void
ggg::basic_ggg<Ch>
::add(const entity_type& ent, const account& acc) {
	if (ent.name() != acc.login()) {
		throw std::invalid_argument("entity name does not match account name");
	}
	this->mkdirs(this->to_relative_path(ent.origin()));
	this->_hierarchy.add(ent);
	this->_accounts.add(acc);
}

template <class Ch>
std::string
ggg::basic_ggg<Ch>
::to_relative_path(const std::string& filename) {
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

template <class Ch>
void
ggg::basic_ggg<Ch>
::mkdirs(std::string relative_path) {
	// split path into components ignoring the last one
	// since it is a file
	size_t i0 = 0, i1 = 0;
	while ((i1 = relative_path.find('/', i0)) != std::string::npos) {
		sys::path p(this->_hierarchy.root(), relative_path.substr(0, i1));
		sys::file_status st(p);
		if (!st.exists()) {
			if (this->verbose()) {
				native_message(std::clog, "Creating _.", p);
			}
			if (-1 == ::mkdir(p, 0755)) {
				throw std::system_error(errno, std::system_category());
			}
		}
		i0 = i1 + 1;
	}
}

template <class Ch>
sys::path
ggg::basic_ggg<Ch>
::account_origin(sys::uid_type uid) {
	sys::path acc_origin;
	if (uid != 0) {
		auto result = this->_hierarchy.find_by_uid(uid);
		if (result == this->_hierarchy.end()) {
			throw std::runtime_error(
				"the user is not allowed to add new entities"
			);
		}
		acc_origin =
			sys::path(GGG_ROOT, "acc", result->name(), "shadow");
	} else {
		acc_origin = GGG_SHADOW;
	}
	return acc_origin;
}

template class ggg::basic_ggg<char>;
