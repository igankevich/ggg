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
typename ggg::basic_ggg<Ch>::entity_type
ggg::basic_ggg<Ch>
::generate(const string_type& name) {
	typedef sys::basic_path<Ch> path_type;
	entity_type result(name.data());
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
	for (const auto& name : names) {
		result.emplace(name.data());
	}
	for (const entity_type& cent : result) {
		entity_type& ent = const_cast<entity_type&>(cent);
		ent.home(path_type(default_home, ent.name()));
		ent.shell(default_shell);
	}
	return result;
}

template class ggg::basic_ggg<char>;
