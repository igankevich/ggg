#include "ggg.hh"

#include <iostream>
#include <system_error>
#include <unistd.h>

#include <unistdx/fs/path>

#include <ggg/bits/to_bytes.hh>
#include <ggg/config.hh>

namespace {

	template <class Ch>
	struct constants;

	template <>
	struct constants<char> {

		static const char*
		home() noexcept {
			return "/home";
		}

		static const char*
		shell() noexcept {
			return "/bin/sh";
		}

	};

	template <>
	struct constants<wchar_t> {

		static const wchar_t*
		home() noexcept {
			return L"/home";
		}

		static const wchar_t*
		shell() noexcept {
			return L"/bin/sh";
		}

	};

}

template <class Ch>
void
ggg::basic_ggg<Ch>
::erase(const string_type& user) {
	bits::wcvt_type cv;
	this->_hierarchy.erase(user.data());
	this->_accounts.erase(bits::to_bytes<char>(cv, user).data());
}

template <class Ch>
bool
ggg::basic_ggg<Ch>
::contains(const string_type& name) {
	bits::wcvt_type cv;
	return this->_hierarchy.find_by_name(name.data()) != this->_hierarchy.end()
	       || this->_accounts.find(bits::to_bytes<char>(cv, name).data()) !=
	       this->_accounts.end();
}

template <class Ch>
typename ggg::basic_ggg<Ch>::entity_type
ggg::basic_ggg<Ch>
::generate(const string_type& name) {
	typedef constants<Ch> traits_type;
	typedef sys::basic_path<Ch> path_type;
	entity_type result = this->_hierarchy.generate(name.data());
	result.home(path_type(traits_type::home(), result.name()));
	result.shell(traits_type::shell());
	return result;
}

template <class Ch>
std::set<typename ggg::basic_ggg<Ch>::entity_type>
ggg::basic_ggg<Ch>
::generate(const std::unordered_set<string_type>& names) {
	typedef constants<Ch> traits_type;
	typedef sys::basic_path<Ch> path_type;
	std::set<entity_type> result;
	this->_hierarchy.generate(
		names.begin(),
		names.end(),
		std::inserter(result, result.begin())
	);
	for (const entity_type& cent : result) {
		entity_type& ent = const_cast<entity_type&>(cent);
		ent.home(path_type(traits_type::home(), ent.name()));
		ent.shell(traits_type::shell());
	}
	return result;
}

template <class Ch>
void
ggg::basic_ggg<Ch>
::add(const entity_type& ent, const account& acc) {
	bits::wcvt_type cv;
	if (bits::to_bytes<char>(cv, ent.name()) != acc.login()) {
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
		bits::wcvt_type cv;
		acc_origin =
			sys::path(
				GGG_ROOT,
				"acc",
				bits::to_bytes<char>(cv, result->name()).data()
			);
	} else {
		acc_origin = GGG_SHADOW;
	}
	return acc_origin;
}

template class ggg::basic_ggg<char>;
template class ggg::basic_ggg<wchar_t>;
