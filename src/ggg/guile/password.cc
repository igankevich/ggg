#include <iostream>
#include <string>

#include <ggg/ctl/password.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/password.hh>
#include <ggg/sec/echo_guard.hh>

namespace {

	SCM
	check_password(SCM string, SCM entropy) {
		using namespace ggg;
		auto min_entropy = scm_to_double(entropy);
		auto s_pw = to_string(string);
		password_match match;
		auto result = !(match.find(s_pw.data()) && match.entropy() < min_entropy);
		return scm_from_bool(result);
	}

	SCM
	get_password(SCM port) {
		using namespace ggg;
		int fd = scm_to_int32(scm_fileno(port));
		if (fd != STDIN_FILENO) {
			scm_throw(scm_from_utf8_symbol("ggg-invalid-argument"),
				scm_from_utf8_string("only port=0 is supported"));
			return SCM_UNSPECIFIED;
		}
		echo_guard g(fd);
		std::string line;
		std::getline(std::cin, line);
		return scm_from_utf8_string(line.data());
	}

}

void
ggg::password_define_procedures() {
	define_procedure("check-password", 2, 0, 0, check_password);
	define_procedure("get-password", 1, 0, 0, get_password);
}

