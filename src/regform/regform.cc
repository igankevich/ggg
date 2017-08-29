#include <sys/types.h>
#include <pwd.h>

#include <security/pam_appl.h>

#include <gtk/gtk.h>

#include <algorithm>
#include <codecvt>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>

#include <config.hh>
#include <pam/pam_handle.hh>
#include <pam/pam_call.hh>
#include <pam/conversation.hh>
#include <ggg/native.hh>
#include <ctl/form.hh>
#include <ctl/account_ctl.hh>

enum struct Conversation_state {
	Authenticating,
	Changing_password,
	Validating,
	Finished
};

Conversation_state state = Conversation_state::Authenticating;
ggg::account recruiter;
ggg::form::container_type fields;
ggg::field_values values;
std::vector<std::string> all_values;
GtkBuilder* builder = nullptr;

std::string
field_id(const ggg::form_field& ff) {
	return "field-" + std::to_string(ff.id());
}

std::string
error_field_id(const ggg::form_field& ff) {
	return "error-" + std::to_string(ff.id());
}

template <class T>
T*
get_widget(const char* name) {
	return (T*)gtk_builder_get_object(builder, name);
}

template <class T>
T*
get_widget(std::string name) {
	return get_widget<T>(name.data());
}

bool
regex_match(const char* string, const std::string& expr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
	std::wregex reg(cv.from_bytes(expr));
	std::wstring value(cv.from_bytes(string));
	return std::regex_match(value, reg);
}

void
register_button_clicked(GtkWidget* widget, gpointer) {
	g_print("Register\n");
}

void
read_entry(const ggg::form_field& ff) {
	GtkEntry* entry = get_widget<GtkEntry>(field_id(ff));
	const char* text = gtk_entry_get_text(entry);
	values[ff] = text;
	all_values.emplace_back(text);
}

bool
validate_entry(const ggg::form_field& ff, const char* text) {
	GtkLabel* errorLabel = get_widget<GtkLabel>(error_field_id(ff));
	bool match = regex_match(text, ff.regex().data());
	const char* errorText = (match || std::strlen(text) == 0) ? "" : "X";
	gtk_label_set_text(errorLabel, errorText);
	return match;
}

bool
validate_login() {
	auto result = std::find_if(
		fields.begin(),
		fields.end(),
		[&] (const ggg::form_field& rhs) {
			return rhs.type() == ggg::field_type::set &&
				rhs.target() == "entity.name";
		}
	);
	bool valid = true;
	if (result != fields.end()) {
		const ggg::form_field& ff = *result;
		std::string value = interpolate(ff.regex(), values);
		GtkLabel* errorLabel = get_widget<GtkLabel>("error");
		struct passwd* pwd = getpwnam(value.data());
		valid &= pwd == nullptr;
		const char* errorText = (valid || value.empty()) ? "" : "User already exists";
		gtk_label_set_text(errorLabel, errorText);
	}
	return valid;
}

void
read_all_entries() {
	all_values.clear();
	const size_t nfields = fields.size();
	for (size_t i=0; i<nfields; ++i) {
		if (fields[i].is_input()) {
			read_entry(fields[i]);
		}
	}
}

bool
validate_all_entries() {
	bool valid = true;
	for (const auto& pair : values) {
		valid &= validate_entry(pair.first, pair.second);
	}
	return valid;
}

void
entry_changed(GtkWidget* entry, gpointer) {
	read_all_entries();
	bool valid = true;
	valid &= validate_all_entries();
	valid &= validate_login();
	GtkWidget* btn = get_widget<GtkWidget>("register");
	gtk_widget_set_sensitive(btn, valid);
}

template <class F>
void
connect_signal(const std::string& objectName, const char* event, F callback) {
	GObject* obj = gtk_builder_get_object(builder, objectName.data());
	g_signal_connect(obj, event, G_CALLBACK(callback), NULL);
}

template <class T>
T*
allocate(size_t n) {
	return reinterpret_cast<T*>(std::malloc(sizeof(T)*n));
}

std::string
read_file(const char* filename) {
	std::stringstream xml;
	xml << std::ifstream(filename).rdbuf();
	return xml.str();
}

std::string
to_gtk_builder_string(const struct pam_message** msgs, size_t n) {
	if (fields.size() < n) {
		throw std::invalid_argument("bad no. of fields");
	}
	std::string entry = read_file(GGG_REGFORM_UI_PATH ".entry");
	std::string button = read_file(GGG_REGFORM_UI_PATH ".button");
	std::string error = read_file(GGG_REGFORM_UI_PATH ".error");
	std::stringstream xml;
	xml << std::ifstream(GGG_REGFORM_UI_PATH ".head").rdbuf();
	for (size_t i=0; i<n; ++i) {
		const struct pam_message* m = msgs[i];
		ggg::format_message(
			xml,
			'?',
			entry.data(),
			m->msg,
			i,
			field_id(fields[i]),
			i,
			error_field_id(fields[i]),
			i
		);
	}
	ggg::format_message(xml, '?', button.data(), "register", "Register", n);
	ggg::format_message(xml, '?', error.data(), "error", n+1);
	xml << std::ifstream(GGG_REGFORM_UI_PATH ".tail").rdbuf();
	return xml.str();
}

int converse(
	int num_msg,
	const struct pam_message** msg,
	struct pam_response** resp,
	void* appdata_ptr
) {
	ggg::pam_errc ret = ggg::pam_errc::success;
	if (state == Conversation_state::Authenticating) {
		if (num_msg == 1 && msg[0]->msg_style == PAM_PROMPT_ECHO_OFF) {
			*resp = allocate<struct pam_response>(1);
			resp[0]->resp = strdup("");
			resp[0]->resp_retcode = 0;
			state = Conversation_state::Validating;
		} else {
			ret = ggg::pam_errc::conversation_error;
		}
	} else if (state == Conversation_state::Changing_password) {
		// TODO
	} else if (state == Conversation_state::Validating) {
		for (int i=0; i<num_msg; ++i) {
			std::clog << "msg[i]->msg=" << msg[i]->msg << std::endl;
		}
		std::string xml = to_gtk_builder_string(msg, num_msg);
		std::clog << "xml=" << xml << std::endl;
		builder = gtk_builder_new();
		gtk_builder_add_from_string(builder, xml.data(), xml.size(), NULL);
		connect_signal("window", "destroy", gtk_main_quit);
		for (int i=0; i<num_msg; ++i) {
			const ggg::form_field& ff = fields[i];
			connect_signal(field_id(ff), "changed", entry_changed);
		}
		connect_signal("register", "clicked", gtk_main_quit);
		gtk_main();
		if (size_t(num_msg) != all_values.size()) {
			throw std::invalid_argument("wrong number of fields");
		}
		struct pam_response* r = allocate<struct pam_response>(num_msg);
		for (int i=0; i<num_msg; ++i) {
			r[i].resp = strdup(all_values[i].data());
			r[i].resp_retcode = 0;
		}
		*resp = r;
		state = Conversation_state::Finished;
	} else {
		// TODO
	}
	return int(ret);
}

int
main(int argc, char* argv[]) {

	if (argc != 2) {
		throw std::invalid_argument("bad username");
	}

	const char* username = argv[1];
	{
		ggg::account_ctl accounts;
		auto result = accounts.find(username);
		if (result == accounts.end()) {
			throw std::invalid_argument("account not found");
		}
		recruiter = *result;
		ggg::form f(recruiter);
		fields = f.fields();
	}

	gtk_init(&argc, &argv);

	ggg::pam_handle pamh;
	ggg::conversation conv(converse);
	ggg::pam::call(::pam_start("sshd", username, &conv, pamh));
	try {
		ggg::pam::call(::pam_authenticate(pamh, 0));
	} catch (const std::system_error& err) {
		if (err.code().value() == PAM_NEW_AUTHTOK_REQD) {
			state = Conversation_state::Changing_password;
			ggg::pam::call(::pam_chauthtok(pamh, 0));
		} else {
			throw;
		}
	}
	ggg::pam::call(::pam_acct_mgmt(pamh, 0));
	ggg::pam::call(::pam_end(pamh, 0));

	return 0;
}

