#include <cstring>

#include <unistdx/ipc/process>
#include <thread>

#include <gtest/gtest.h>

#include <ggg/config.hh>
#include <ggg/pam/call.hh>
#include <ggg/pam/conversation.hh>
#include <ggg/pam/handle.hh>
#include <ggg/pam/pam_handle.hh>
#include <ggg/sec/argon2.hh>
#include <ggg/sec/password.hh>
#include <ggg/test/clean_database.hh>
#include <ggg/proto/daemon_environment.hh>

const char* testuser_password = "jae3wahQue";
const char* testuser_new_password = "gah9nuyeGh";

ggg::entity
testuser_entity(std::string str="testuser:x:2000:2000:halt:/sbin:/sbin/halt") {
    ggg::entity ent;
    std::stringstream(str) >> ent;
    return ent;
}

ggg::account
testuser_account() {
    using namespace ggg;
    account acc("testuser");
    argon2_password_hash hash;
    acc.set_password(hash(testuser_password));
    return acc;
}

void
add_testuser_with_password(Clean_database& db, const char* password_id) {
    using namespace ggg;
    db.Database::insert(testuser_entity());
    db.Database::insert(testuser_account());
}

void
add_testuser_with_password(const char* password_id) {
    Clean_database db;
    add_testuser_with_password(db, password_id);
}

void
add_testuser_without_password(Clean_database& db) {
    db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
}

void
add_testuser_without_password() {
    Clean_database db;
    add_testuser_without_password(db);
}

template <class T>
T*
allocate(size_t n) {
    return reinterpret_cast<T*>(std::malloc(sizeof(T)*n));
}

int
empty_conversation(
    int,
    const struct pam_message**,
    struct pam_response**,
    void*
) {
    return static_cast<int>(pam::errc::success);
}

int
default_conversation(
    int num_msg,
    const struct pam_message** msg,
    struct pam_response** resp,
    void* appdata_ptr
) {
    pam::errc ret;
    if (num_msg == 1 && msg[0]->msg_style == PAM_PROMPT_ECHO_OFF) {
        *resp = allocate<struct pam_response>(1);
        resp[0]->resp = strdup(testuser_password);
        resp[0]->resp_retcode = 0;
        ret = pam::errc::success;
    } else {
        ret = pam::errc::conversation_error;
    }
    return int(ret);
}

enum class Conversation_state {
    Authenticate,
    Get_old_password,
    Get_new_password,
    Authenticate_with_new_password
};

Conversation_state conversation_state = Conversation_state::Authenticate;

int
conversation_with_state(
    int num_msg,
    const struct pam_message** msg,
    struct pam_response** resp,
    void* appdata_ptr
) {
    pam::errc ret;
    if (num_msg == 1 && msg[0]->msg_style == PAM_PROMPT_ECHO_OFF) {
        *resp = allocate<struct pam_response>(1);
        const char* password = testuser_password;
        switch (conversation_state) {
            case Conversation_state::Authenticate:
                password = testuser_password;
                break;
            case Conversation_state::Get_old_password:
                password = testuser_password;
                conversation_state = Conversation_state::Get_new_password;
                break;
            case Conversation_state::Get_new_password:
                password = testuser_new_password;
                break;
            case Conversation_state::Authenticate_with_new_password:
                password = testuser_new_password;
                break;
            default:
                password = testuser_password;
                break;
        }
        resp[0]->resp = strdup(password);
        resp[0]->resp_retcode = 0;
        ret = pam::errc::success;
    } else {
        ret = pam::errc::conversation_error;
    }
    return int(ret);
}

TEST(pam, start_end) {
    pam::handle h;
    pam::conversation conv(empty_conversation);
    h.start("ggg", "testuser", conv);
    h.open_session();
    h.close_session();
    h.end();
}

TEST(pam, authenticate_without_database) {
    using namespace ggg;
    using pam::handle;
    std::remove(GGG_ENTITIES_PATH);
    std::remove(GGG_ACCOUNTS_PATH);
    pam::handle h;
    pam::conversation conv(default_conversation);
    h.start("ggg", "testuser", conv);
    try {
        h.authenticate();
    } catch (const std::system_error& err) {
        EXPECT_EQ(pam::errc::permission_denied, pam::errc(err.code().value()))
            << err.what();
    }
    h.end();
}

TEST(pam, authenticate_without_password) {
    add_testuser_without_password();
    pam::handle h;
    pam::conversation conv(default_conversation);
    h.start("ggg", "testuser", conv);
    try {
        h.authenticate();
    } catch (const std::system_error& err) {
        EXPECT_EQ(
            pam::errc::permission_denied,
            pam::errc(err.code().value())
        ) << err.what();
    }
    h.end();
}

TEST(pam, authenticate_with_password) {
    add_testuser_with_password("6");
    pam::handle h;
    pam::conversation conv(default_conversation);
    h.start("ggg", "testuser", conv);
    h.authenticate();
    h.end();
}

TEST(pam, authenticate_with_expired_password) {
    {
        Clean_database db;
        add_testuser_with_password(db, "6");
        db.expire_password("testuser");
    }
    pam::handle h;
    pam::conversation conv(default_conversation);
    h.start("ggg", "testuser", conv);
    h.authenticate();
    try {
        h.verify_account();
    } catch (const std::system_error& err) {
        EXPECT_EQ(
            pam::errc::new_password_required,
            pam::errc(err.code().value())
        ) << err.what();
    }
    h.end();
}

TEST(pam, authenticate_with_expired_password_and_change_it) {
    {
        Clean_database db;
        add_testuser_with_password(db, "6");
        db.expire_password("testuser");
    }
    conversation_state = Conversation_state::Authenticate;
    pam::handle h;
    pam::conversation conv(conversation_with_state);
    h.start("ggg", "testuser", conv);
    h.authenticate();
    try {
        h.verify_account();
    } catch (const std::system_error& err) {
        EXPECT_EQ(
            pam::errc::new_password_required,
            pam::errc(err.code().value())
        ) << err.what();
        conversation_state = Conversation_state::Get_old_password;
        h.change_password();
        conversation_state = Conversation_state::Authenticate_with_new_password;
        h.authenticate();
        h.verify_account();
    }
    h.end();
}

TEST(pam, authenticate_with_valid_account) {
    add_testuser_with_password("6");
    pam::handle h;
    pam::conversation conv(default_conversation);
    h.start("ggg", "testuser", conv);
    h.authenticate();
    h.verify_account();
    h.end();
}

TEST(pam, authenticate_with_suspended_account) {
    {
        Clean_database db;
        add_testuser_with_password(db, "6");
        db.deactivate("testuser");
    }
    pam::handle h;
    pam::conversation conv(default_conversation);
    h.start("ggg", "testuser", conv);
    h.authenticate();
    try {
        h.verify_account();
    } catch (const std::system_error& err) {
        EXPECT_EQ(
            pam::errc::permission_denied,
            pam::errc(err.code().value())
        ) << err.what();
    }
    h.end();
}

TEST(pam, authenticate_with_expired_account) {
    {
        Clean_database db;
        add_testuser_with_password(db, "6");
        db.expire("testuser");
    }
    pam::handle h;
    pam::conversation conv(default_conversation);
    h.start("ggg", "testuser", conv);
    h.authenticate();
    try {
        h.verify_account();
    } catch (const std::system_error& err) {
        EXPECT_EQ(
            pam::errc::account_expired,
            pam::errc(err.code().value())
        ) << err.what();
    }
    h.end();
}

TEST(pam, change_password_and_authenticate) {
    conversation_state = Conversation_state::Get_old_password;
    add_testuser_with_password("6");
    pam::handle h;
    pam::conversation conv(conversation_with_state);
    h.start("ggg", "testuser", conv);
    h.change_password();
    conversation_state = Conversation_state::Authenticate_with_new_password;
    h.authenticate();
    h.verify_account();
    h.end();
}

TEST(pam, change_password_of_expired_account) {
    {
        Clean_database db;
        add_testuser_with_password(db, "6");
        db.expire("testuser");
    }
    conversation_state = Conversation_state::Get_old_password;
    pam::handle h;
    pam::conversation conv(conversation_with_state);
    h.start("ggg", "testuser", conv);
    try {
        h.change_password();
    } catch (const std::system_error& err) {
        EXPECT_EQ(pam::errc::account_expired, pam::errc(err.code().value()))
            << err.what();
    }
    h.end();
}

TEST(pam, authenticate_with_inactive_account) {
    {
        Clean_database db;
        db.Database::insert(testuser_entity());
        auto acc = testuser_account();
        auto now = ggg::account::clock_type::now();
        acc.max_inactive(std::chrono::hours(24));
        acc.last_active(now-std::chrono::hours(48));
        db.Database::insert(acc);
    }
    pam::handle h;
    pam::conversation conv(default_conversation);
    h.start("ggg", "testuser", conv);
    h.authenticate();
    try {
        h.verify_account();
    } catch (const std::system_error& err) {
        EXPECT_EQ(
            pam::errc::account_expired,
            pam::errc(err.code().value())
        ) << err.what();
    }
    h.end();
}

int
main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new DaemonEnvironment);
    return RUN_ALL_TESTS();
}
