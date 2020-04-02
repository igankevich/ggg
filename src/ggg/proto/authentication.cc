#include <sstream>

#include <ggg/core/database.hh>
#include <ggg/proto/authentication.hh>
#include <ggg/sec/argon2.hh>
#include <ggg/sec/password.hh>

void ggg::PAM_kernel::run() {
    const auto user = this->_name.data();
    account acc;
    Database db(Database::File::Accounts, Database::Flag::Read_write);
    db.prepare_message();
    if (steps() & (Auth | Account | Password)) {
        auto st = db.find_account(user);
        if (st.step() == sqlite::errc::done) {
            std::stringstream msg;
            msg << "invalid user \"" << user << "\"";
            throw std::invalid_argument(msg.str());
        }
        st >> acc; st.close();
    }
    if (steps() & Auth) {
        if (!verify_password(acc.password(), this->_password.data())) {
            std::stringstream msg;
            msg << "permission denied for user \"" << user << "\"";
            throw std::invalid_argument(msg.str());
        }
        db.message(user, "authenticated");
        steps_result(steps_result() | Auth);
    }
    if (steps() & Account) {
        const auto now = db.timestamp();
        sys::u32 error = 0;
        if (acc.has_been_suspended()) { error |= Error::Suspended; }
        if (acc.has_expired(now)) { error |= Error::Expired; }
        if (acc.is_inactive(now)) { error |= Error::Inactive; }
        if (acc.password_has_expired(now)) { error |= Error::Password_expired; }
        steps_result(steps_result() | error);
        if (error == 0) {
            acc.last_active(now);
            db.set_last_active(acc, now);
            steps_result(steps_result() | Account);
        }
    }
    if (steps() & Password) {
        const auto now = db.timestamp();
        sys::u32 error = 0;
        if (acc.has_been_suspended()) { error |= Error::Suspended; }
        if (acc.has_expired(now)) { error |= Error::Expired; }
        if (acc.is_inactive(now)) { error |= Error::Inactive; }
        steps_result(steps_result() | error);
        if (error == 0) {
            const auto& client = client_credentials();
            if (client.uid != 0 &&
                !verify_password(acc.password(), this->_old_password.data())) {
                std::stringstream msg;
                msg << "permission denied for user \"" << user << "\"";
                throw std::invalid_argument(msg.str());
            }
            validate_password(this->_password, this->_min_entropy);
            init_sodium();
            argon2_password_hash hash;
            acc.set_password(hash(this->_password));
            db.set_password(acc);
            steps_result(steps_result() | Password);
        }
    }
    if (steps() & Open_session) {
        db.message(user, "session opened for %s", this->_service.data());
        steps_result(steps_result() | Open_session);
    }
    if (steps() & Close_session) {
        db.message(user, "session closed for %s", this->_service.data());
        steps_result(steps_result() | Close_session);
    }
}

void ggg::PAM_kernel::read(sys::byte_buffer& buf) {
    buf.read(this->_result);
    buf.read(this->_steps_result);
    buf.read(this->_name);
    buf.read(this->_password);
    buf.read(this->_old_password);
    buf.read(this->_service);
    buf.read(this->_min_entropy);
    buf.read(this->_steps);
}

void ggg::PAM_kernel::write(sys::byte_buffer& buf) {
    buf.write(this->_result);
    buf.write(this->_steps_result);
    buf.write(this->_name);
    buf.write(this->_password);
    buf.write(this->_old_password);
    buf.write(this->_service);
    buf.write(this->_min_entropy);
    buf.write(this->_steps);
}
