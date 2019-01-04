#include <cstring>
#include <ggg/ctl/password.hh>
#include <ggg/pam/conversation.hh>
#include <ggg/pam/pam_call.hh>
#include <ggg/pam/pam_handle.hh>
#include <ggg/test/clean_database.hh>
#include <gtest/gtest.h>

const char* testuser_password = "jae3wahQue";
const char* testuser_new_password = "gah9nuyeGh";

void
add_testuser_with_password(Clean_database& db, const char* password_id) {
	db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
	ggg::account acc("testuser");
	ggg::secure_string encrypted = ggg::encrypt(
		testuser_password,
		ggg::account::password_prefix(ggg::generate_salt(), password_id, 0)
	);
	acc.set_password(encrypted);
	db.update(acc);
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

void
add_testuser_with_password_and_expired_account(const char* password_id) {
	Clean_database db;
	add_testuser_with_password(db, password_id);
	db.expire("testuser");
}

void
add_testuser_with_expired_password(const char* password_id) {
	Clean_database db;
	add_testuser_with_password(db, password_id);
	db.expire_password("testuser");
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
	return static_cast<int>(ggg::pam_errc::success);
}

int
default_conversation(
	int num_msg,
	const struct pam_message** msg,
	struct pam_response** resp,
	void* appdata_ptr
) {
	ggg::pam_errc ret;
	if (num_msg == 1 && msg[0]->msg_style == PAM_PROMPT_ECHO_OFF) {
		*resp = allocate<struct pam_response>(1);
		resp[0]->resp = strdup(testuser_password);
		resp[0]->resp_retcode = 0;
		ret = ggg::pam_errc::success;
	} else {
		ret = ggg::pam_errc::conversation_error;
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
	ggg::pam_errc ret;
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
		ret = ggg::pam_errc::success;
	} else {
		ret = ggg::pam_errc::conversation_error;
	}
	return int(ret);
}

TEST(pam, start_end) {
	ggg::pam_handle pamh;
	ggg::conversation conv(empty_conversation);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, authenticate_without_database) {
	std::remove(GGG_ENTITIES_PATH);
	ggg::pam_handle pamh;
	ggg::conversation conv(default_conversation);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	try {
		ggg::pam::call(::pam_authenticate(pamh, 0));
	} catch (const std::system_error& err) {
		EXPECT_EQ(
			ggg::pam_errc::authentication_error,
			ggg::pam_errc(err.code().value())
		) << err.what();
	}
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, authenticate_testuser_without_password) {
	add_testuser_without_password();
	ggg::pam_handle pamh;
	ggg::conversation conv(default_conversation);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	try {
		ggg::pam::call(::pam_authenticate(pamh, 0));
	} catch (const std::system_error& err) {
		EXPECT_EQ(
			ggg::pam_errc::permission_denied,
			ggg::pam_errc(err.code().value())
		) << err.what();
	}
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, authenticate_testuser_with_password) {
	add_testuser_with_password("6");
	ggg::pam_handle pamh;
	ggg::conversation conv(default_conversation);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	ggg::pam::call(::pam_authenticate(pamh, 0));
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, authenticate_testuser_with_password_and_valid_account) {
	add_testuser_with_password("6");
	ggg::pam_handle pamh;
	ggg::conversation conv(default_conversation);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	ggg::pam::call(::pam_authenticate(pamh, 0));
	ggg::pam::call(::pam_acct_mgmt(pamh, 0));
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, authenticate_testuser_with_password_and_expired_account) {
	add_testuser_with_password_and_expired_account("6");
	ggg::pam_handle pamh;
	ggg::conversation conv(default_conversation);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	ggg::pam::call(::pam_authenticate(pamh, 0));
	try {
		ggg::pam::call(::pam_acct_mgmt(pamh, 0));
	} catch (const std::system_error& err) {
		EXPECT_EQ(
			ggg::pam_errc::account_expired,
			ggg::pam_errc(err.code().value())
		) << err.what();
	}
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, authenticate_testuser_with_expired_password) {
	add_testuser_with_expired_password("6");
	ggg::pam_handle pamh;
	ggg::conversation conv(default_conversation);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	ggg::pam::call(::pam_authenticate(pamh, 0));
	try {
		ggg::pam::call(::pam_acct_mgmt(pamh, 0));
	} catch (const std::system_error& err) {
		EXPECT_EQ(
			ggg::pam_errc::new_password_required,
			ggg::pam_errc(err.code().value())
		) << err.what();
	}
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, authenticate_testuser_with_expired_password_and_change_it) {
	conversation_state = Conversation_state::Authenticate;
	add_testuser_with_expired_password("6");
	ggg::pam_handle pamh;
	ggg::conversation conv(conversation_with_state);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	ggg::pam::call(::pam_authenticate(pamh, 0));
	try {
		ggg::pam::call(::pam_acct_mgmt(pamh, 0));
	} catch (const std::system_error& err) {
		EXPECT_EQ(
			ggg::pam_errc::new_password_required,
			ggg::pam_errc(err.code().value())
		) << err.what();
		conversation_state = Conversation_state::Get_old_password;
		ggg::pam::call(::pam_chauthtok(pamh, 0));
		conversation_state = Conversation_state::Authenticate_with_new_password;
		ggg::pam::call(::pam_authenticate(pamh, 0));
		ggg::pam::call(::pam_acct_mgmt(pamh, 0));
	}
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, change_password_and_authenticate) {
	conversation_state = Conversation_state::Get_old_password;
	add_testuser_with_password("6");
	ggg::pam_handle pamh;
	ggg::conversation conv(conversation_with_state);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	ggg::pam::call(::pam_chauthtok(pamh, 0));
	conversation_state = Conversation_state::Authenticate_with_new_password;
	ggg::pam::call(::pam_authenticate(pamh, 0));
	ggg::pam::call(::pam_acct_mgmt(pamh, 0));
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, change_password_of_expired_account) {
	conversation_state = Conversation_state::Get_old_password;
	add_testuser_with_password_and_expired_account("6");
	ggg::pam_handle pamh;
	ggg::conversation conv(conversation_with_state);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	try {
		ggg::pam::call(::pam_chauthtok(pamh, 0));
	} catch (const std::system_error& err) {
		EXPECT_EQ(
			ggg::pam_errc::account_expired,
			ggg::pam_errc(err.code().value())
		) << err.what();
	}
	ggg::pam::call(::pam_end(pamh, 0));
}

TEST(pam, authenticate_then_account_then_session) {
	add_testuser_with_password("6");
	ggg::pam_handle pamh;
	ggg::conversation conv(default_conversation);
	ggg::pam::call(::pam_start("ggg", "testuser", &conv, pamh));
	ggg::pam::call(::pam_authenticate(pamh, 0));
	ggg::pam::call(::pam_acct_mgmt(pamh, 0));
	ggg::pam::call(::pam_open_session(pamh, 0));
	ggg::pam::call(::pam_close_session(pamh, 0));
	ggg::pam::call(::pam_end(pamh, 0));
}
