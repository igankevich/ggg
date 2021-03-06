#include <iostream>
#include <string>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/password.hh>
#include <ggg/sec/argon2.hh>
#include <ggg/sec/echo_guard.hh>
#include <ggg/sec/password.hh>

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
        init_sodium();
        argon2_password_hash hash;
        return scm_from_utf8_string(hash(to_secure_string(password)).data());
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
            init_sodium();
            return scm_from_bool(verify_password(
                acc.password(),
                to_secure_string(password)
            ));
        } catch (const std::exception& err) {
            guile_throw(err);
        }
        return SCM_UNSPECIFIED;
    }

    SCM
    scm_verify_password(SCM hashed_password, SCM password) {
        using namespace ggg;
        try {
            return scm_from_bool(verify_password(
                to_string(hashed_password),
                to_secure_string(password)
            ));
        } catch (const std::exception& err) {
            guile_throw(err);
        }
        return SCM_UNSPECIFIED;
    }

    SCM
    scm_change_password(SCM username, SCM old_password, SCM new_password) {
        const double min_entropy = 30.0;
        using namespace ggg;
        try {
            auto s_username = to_secure_string(username);
            auto s_old_password = to_secure_string(old_password);
            auto s_new_password = to_secure_string(new_password);
            account acc;
            Database db(Database::File::Accounts, Database::Flag::Read_write);
            Transaction tr(db);
            db.prepare_message();
            auto st = db.find_account(s_username.data());
            if (st.step() == sqlite::errc::done) { throw std::invalid_argument("bad name"); }
            st >> acc; st.close();
            if (!verify_password(acc.password(), s_old_password.data())) {
                throw std::invalid_argument("permission denied");
            }
            validate_password(s_new_password, min_entropy);
            init_sodium();
            argon2_password_hash hash;
            acc.set_password(hash(s_new_password));
            db.set_password(acc);
            tr.commit();
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
    define_procedure("verify-password", 2, 0, 0, scm_verify_password);
    define_procedure("change-password", 3, 0, 0, scm_change_password);
}
