#include <iostream>
#include <string>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
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
		auto result = match.find(s_pw.data()) && !(match.entropy() < min_entropy);
		return scm_from_bool(result);
	}

	SCM
	get_password(SCM port) {
		using namespace ggg;
		int fd = scm_to_int32(scm_fileno(port));
		if (fd != STDIN_FILENO) {
            guile_throw("only port=0 is supported");
			return SCM_UNSPECIFIED;
		}
		echo_guard g(fd);
		std::string line;
		std::getline(std::cin, line);
		return scm_from_utf8_string(line.data());
	}

	SCM
	password_hash(SCM password) {
		using namespace ggg;
		std::string password_id = "6";
		unsigned int num_rounds = 0;
		auto prefix = account::password_prefix(generate_salt(), password_id, num_rounds);
		auto encrypted = encrypt(to_string(password).data(), prefix);
		return scm_from_utf8_string(encrypted.data());
	}

	SCM
	authenticate(SCM username, SCM password) {
		using namespace ggg;
        try {
            Database db(Database::File::Accounts, Database::Flag::Read_only);
            auto rstr = db.find_account(to_string(username).data());
            account_iterator first(rstr), last;
            if (first == last) { return scm_from_bool(false); }
            account acc = *first;
            auto encrypted = encrypt(to_string(password).data(), acc.password_prefix());
            return scm_from_bool(acc.password() == encrypted);
        } catch (const std::exception& err) {
			guile_throw(err);
        }
        return SCM_UNSPECIFIED;
    }

}

void
ggg::password_define_procedures() {
    define_procedure("check-password", 2, 0, 0, check_password);
    define_procedure("get-password", 1, 0, 0, get_password);
    define_procedure("password-hash", 1, 0, 0, password_hash);
    define_procedure("authenticate", 2, 0, 0, authenticate);
}

