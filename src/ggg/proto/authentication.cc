#include <ggg/core/database.hh>
#include <ggg/proto/authentication.hh>
#include <ggg/sec/argon2.hh>
#include <ggg/sec/password.hh>

#include <iostream>

void ggg::PAM_kernel::run() {
    const auto user = this->_name.data();
    std::clog << "client_credentials().uid=" << client_credentials().uid << std::endl;
    account acc;
    Database db(Database::File::Accounts, Database::Flag::Read_write);
    db.prepare_message();
    if (steps() & (Auth | Account | Password)) {
        auto st = db.find_account(user);
        if (st.step() == sqlite::errc::done) { throw std::invalid_argument("bad name"); }
        st >> acc; st.close();
    }
    if (steps() & Auth) {
        if (!verify_password(acc.password(), this->_password.data())) {
            throw std::invalid_argument("permission denied");
        }
        db.message(user, "authenticated");
        result(result() | Auth);
        std::string argon2_prefix = "$argon2id$";
        if (acc.password().compare(0, argon2_prefix.size(), argon2_prefix) != 0) {
            init_sodium();
            argon2_password_hash hash;
            acc.set_password(hash(this->_password));
            db.set_password(acc);
            db.message(user, "migrated to argon2");
        }
    }
    if (steps() & Account) {
        const auto now = db.timestamp();
        sys::u32 error = 0;
        if (acc.has_been_suspended()) { error |= Error::Suspended; }
        if (acc.has_expired(now)) { error |= Error::Expired; }
        if (acc.is_inactive(now)) { error |= Error::Inactive; }
        if (acc.password_has_expired(now)) { error |= Error::Password_expired; }
        result(result() | error);
        if (error == 0) {
            acc.last_active(now);
            db.set_last_active(acc, now);
            result(result() | Account);
        }
    }
    if (steps() & Password) {
        const auto now = db.timestamp();
        sys::u32 error = 0;
        if (acc.has_been_suspended()) { error |= Error::Suspended; }
        if (acc.has_expired(now)) { error |= Error::Expired; }
        if (acc.is_inactive(now)) { error |= Error::Inactive; }
        result(result() | error);
        if (error == 0) {
            const auto& client = client_credentials();
            if (client.uid != 0 &&
                !verify_password(acc.password(), this->_old_password.data())) {
                std::clog << "111=" << 111 << std::endl;
                throw std::invalid_argument("permission denied");
            }
            validate_password(this->_password, this->_min_entropy);
            init_sodium();
            argon2_password_hash hash;
            acc.set_password(hash(this->_password));
            db.set_password(acc);
            result(result() | Password);
        }
    }
    if (steps() & Open_session) {
        db.message(user, "session opened for %s", this->_service.data());
        result(result() | Open_session);
    }
    if (steps() & Close_session) {
        db.message(user, "session closed for %s", this->_service.data());
        result(result() | Close_session);
    }
}

void ggg::PAM_kernel::read(byte_buffer& buf) {
    buf.read(this->_name);
    buf.read(this->_password);
    buf.read(this->_old_password);
    buf.read(this->_service);
    buf.read(this->_min_entropy);
    buf.read(this->_steps);
}

void ggg::PAM_kernel::write(byte_buffer& buf) {
    buf.write(this->_name);
    buf.write(this->_password);
    buf.write(this->_old_password);
    buf.write(this->_service);
    buf.write(this->_min_entropy);
    buf.write(this->_steps);
}
