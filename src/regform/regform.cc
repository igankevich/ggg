#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <security/pam_appl.h>

#include <gtk/gtk.h>

#include <algorithm>
#include <codecvt>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <locale>
#include <mutex>
#include <regex>
#include <sstream>
#include <thread>
#include <vector>

#include <config.hh>
#include <pam/pam_handle.hh>
#include <pam/pam_call.hh>
#include <pam/conversation.hh>
#include <core/native.hh>
#include <ctl/form.hh>
#include <ctl/account_ctl.hh>

#include <unistdx/process>
#include <unistdx/base/check>

enum struct Conversation_state {
	Authenticating,
	Changing_password,
	Validating,
	Registering,
	Finished,
	Quit
};

enum struct Form_type {
	Registration,
	Login
};

Form_type form_type = Form_type::Login;
Conversation_state state = Conversation_state::Authenticating;
ggg::account recruiter;
ggg::form::container_type fields;
ggg::field_values values;
std::vector<std::string> all_values;
GtkWidget* registerPageGrid = nullptr;
GtkWidget* loginErrorLabel = nullptr;
GtkWidget* registerButton = nullptr;
GtkWidget* window = nullptr;
GtkWidget* loginEntry = nullptr;
GtkWidget* passwordEntry = nullptr;
std::vector<GtkWidget*> all_entries;
std::vector<GtkWidget*> all_error_labels;
std::thread pam_thread;
std::recursive_mutex pam_mutex;
std::condition_variable_any pam_cv;
const char* username = nullptr;
bool all_valid = false;

bool
regex_match(const char* string, const std::string& expr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
	std::wregex reg(cv.from_bytes(expr));
	std::wstring value(cv.from_bytes(string));
	return std::regex_match(value, reg);
}

void
register_user(GtkButton*, gpointer) {
	if (!all_valid) return;
	// disable the button
	gtk_widget_set_sensitive(GTK_WIDGET(registerButton), FALSE);
	gtk_button_set_label(GTK_BUTTON(registerButton), ggg::native("Please wait...").data());
	// proceed PAM conversation
	pam_cv.notify_one();
}

void
read_entry(GtkWidget* entry, const ggg::form_field& ff) {
	const char* text = gtk_entry_get_text(GTK_ENTRY(entry));
	values[ff] = text;
	all_values.emplace_back(text);
}

bool
validate_entry(
	const ggg::form_field& ff,
	const char* text,
	GtkWidget* errorLabel
) {
	const bool match = regex_match(text, ff.regex().data());
	const char* errorText = (match || std::strlen(text) == 0) ? "" : "X";
	gtk_label_set_text(GTK_LABEL(errorLabel), errorText);
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
	const char* errorText = "";
	if (result != fields.end()) {
		const ggg::form_field& ff = *result;
		std::string value = interpolate(ff.regex(), values);
		struct passwd* pwd = getpwnam(value.data());
		valid &= pwd == nullptr;
		errorText = (valid || value.empty()) ? "" : "User already exists";
	}
	gtk_label_set_text(GTK_LABEL(loginErrorLabel), errorText);
	return valid;
}

void
read_all_entries() {
	all_values.clear();
	const size_t nfields = fields.size();
	int entry_no = 0;
	for (size_t i=0; i<nfields; ++i) {
		if (fields[i].is_input()) {
			read_entry(all_entries[entry_no], fields[i]);
			++entry_no;
		}
	}
}

bool
validate_all_entries() {
	bool valid = true;
	int entry_no = 0;
	for (const ggg::form_field& ff : fields) {
		if (ff.is_input()) {
			const char* value = values[ff];
			valid &= validate_entry(ff, value, all_error_labels[entry_no]);
			++entry_no;
		}
	}
	return valid;
}

void
entry_changed(GtkWidget*, gpointer) {
	read_all_entries();
	bool valid = true;
	valid &= validate_all_entries();
	valid &= validate_login();
	all_valid = valid;
	gtk_widget_set_sensitive(registerButton, valid);
}

template <class T>
T*
allocate(size_t n) {
	return reinterpret_cast<T*>(std::malloc(sizeof(T)*n));
}

std::vector<GtkWidget*>
new_widget(const struct pam_message* msg, const ggg::form_field& ff) {
	const bool is_input = msg->msg_style == PAM_PROMPT_ECHO_ON ||
		msg->msg_style == PAM_PROMPT_ECHO_OFF;
	std::vector<GtkWidget*> ret;
	if (is_input) {
		GtkWidget* label = gtk_label_new(msg->msg);
		gtk_label_set_xalign(GTK_LABEL(label), 1.f);
		GtkWidget* entry = gtk_entry_new();
		if (msg->msg_style == PAM_PROMPT_ECHO_OFF) {
			gtk_entry_set_input_purpose(
				GTK_ENTRY(entry),
				GTK_INPUT_PURPOSE_PASSWORD
			);
			gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
		}
		GtkWidget* errorLabel = gtk_label_new("");
		g_signal_connect(entry, "changed", G_CALLBACK(entry_changed), nullptr);
		ret.push_back(label);
		ret.push_back(entry);
		ret.push_back(errorLabel);
		all_entries.push_back(entry);
		all_error_labels.push_back(errorLabel);
	} else if (msg->msg_style == PAM_ERROR_MSG) {
		GtkWidget* label = gtk_label_new(nullptr);
		std::string text_in_bold;
		text_in_bold.append("<b>");
		text_in_bold.append(msg->msg);
		text_in_bold.append("</b>");
		gtk_label_set_markup(GTK_LABEL(label), text_in_bold.data());
		ret.push_back(label);
	} else {
		GtkWidget* label = gtk_label_new(msg->msg);
		ret.push_back(label);
	}
	return ret;
}

GtkWidget*
new_widget_form(const struct pam_message** msg, int num_msg) {
	all_entries.clear();
	all_error_labels.clear();
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
	for (int i=0; i<num_msg; ++i) {
		const ggg::form_field& ff = fields[i];
		std::vector<GtkWidget*> widgets = new_widget(msg[i], ff);
		const int nwidgets = widgets.size();
		for (int j=0; j<nwidgets; ++j) {
			gtk_grid_attach(GTK_GRID(grid), widgets[j], j, i, 1, 1);
		}
	}
	// widgets
	registerButton = gtk_button_new_with_label(ggg::native("Register").data());
	gtk_widget_set_sensitive(registerButton, FALSE);
	loginErrorLabel = gtk_label_new("");
	// layout
	gtk_grid_attach(GTK_GRID(grid), registerButton, 1, num_msg, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), loginErrorLabel, 0, num_msg+1, 3, 1);
	return grid;
}

gboolean
show_message_box(gpointer data) {
	const struct pam_message* msg = reinterpret_cast<const struct pam_message*>(data);
	GtkWidget* dialog = gtk_message_dialog_new(
		GTK_WINDOW(window),
		GTK_DIALOG_MODAL,
		msg->msg_style == PAM_ERROR_MSG ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK,
		"%s",
		msg->msg
	);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	pam_cv.notify_one();
	return FALSE;
}

gboolean
show_registration_form(gpointer data) {
	typedef std::pair<const struct pam_message**,int> pair_type;
	pair_type* pair = reinterpret_cast<pair_type*>(data);
	const struct pam_message** msg = pair->first;
	int num_msg = pair->second;
	GtkWidget* newRegisterForm = new_widget_form(msg, num_msg);
	gtk_container_foreach(
		GTK_CONTAINER(registerPageGrid),
		(GtkCallback)gtk_widget_destroy,
		nullptr
	);
	gtk_grid_attach(GTK_GRID(registerPageGrid), newRegisterForm, 0, 0, 1, 1);
	// signals
	g_signal_connect(
		registerButton,
		"clicked",
		G_CALLBACK(register_user),
		nullptr
	);
	gtk_widget_show_all(registerPageGrid);
	return FALSE;
}

int converse_log_in(
	int num_msg,
	const struct pam_message** msg,
	struct pam_response** resp,
	void* appdata_ptr
) {
	ggg::pam_errc ret;
	if (num_msg == 1 && msg[0]->msg_style == PAM_PROMPT_ECHO_OFF) {
		const char* password = gtk_entry_get_text(GTK_ENTRY(passwordEntry));
		*resp = allocate<struct pam_response>(1);
		resp[0]->resp = strdup(password);
		resp[0]->resp_retcode = 0;
		ret = ggg::pam_errc::success;
	} else {
		ret = ggg::pam_errc::conversation_error;
	}
	return int(ret);
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
		ret = ggg::pam_errc::conversation_error;
	} else if (state == Conversation_state::Validating) {
		auto pair = std::make_pair(msg, num_msg);
		gdk_threads_add_idle(show_registration_form, &pair);
		std::unique_lock<std::recursive_mutex> lock(pam_mutex);
		pam_cv.wait(lock);
		if (state == Conversation_state::Quit) {
			*resp = nullptr;
			ret = ggg::pam_errc::conversation_error;
		} else {
			if (size_t(num_msg) != all_values.size()) {
				*resp = nullptr;
				ret = ggg::pam_errc::conversation_error;
			} else {
				struct pam_response* r = allocate<struct pam_response>(num_msg);
				for (int i=0; i<num_msg; ++i) {
					r[i].resp = strdup(all_values[i].data());
					r[i].resp_retcode = 0;
				}
				*resp = r;
				state = Conversation_state::Registering;
			}
		}
	} else if (state == Conversation_state::Registering) {
		if (num_msg == 1 && (msg[0]->msg_style == PAM_ERROR_MSG ||
					msg[0]->msg_style == PAM_TEXT_INFO))
		{
			gdk_threads_add_idle(show_message_box, const_cast<struct pam_message*>(msg[0]));
			std::unique_lock<std::recursive_mutex> lock(pam_mutex);
			pam_cv.wait(lock);
			if (msg[0]->msg_style == PAM_TEXT_INFO) {
				state = Conversation_state::Finished;
			} else {
				state = Conversation_state::Validating;
			}
		} else {
			ret = ggg::pam_errc::conversation_error;
		}
	}
	return int(ret);
}

void
parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = ::getopt(argc, argv, "l")) != -1) {
		if (opt == 'l') {
			form_type = Form_type::Login;
		}
	}
	if (::optind == argc) {
		throw std::invalid_argument("bad username");
	}
}

void
load_form(const char* username) {
	ggg::account_ctl accounts;
	auto result = accounts.find(username);
	if (result == accounts.end()) {
		throw std::invalid_argument("account not found");
	}
	recruiter = *result;
	ggg::form f(recruiter);
	fields = f.fields();
}

gboolean
quit_application(gpointer data) {
	gtk_widget_destroy(window);
	return FALSE;
}

template <class ConverseFunc>
void
do_pam(const char* username, ConverseFunc func) {
	ggg::pam_handle pamh;
	ggg::conversation conv(func);
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
	ggg::pam::call(::pam_open_session(pamh, 0));
	ggg::pam::call(::pam_end(pamh, 0));
	if (window) {
		gdk_threads_add_idle(quit_application, nullptr);
	}
}

void
fork_exec(const char* username) {
	struct ::passwd* pw = ::getpwnam(username);
	if (!pw) {
		std::clog << "user " << username << " not found via NSS." << std::endl;
		std::exit(1);
	}
	const char* pwd = pw->pw_dir;
	if (-1 == ::chdir(pwd)) {
		pwd = "/";
		::chdir(pwd);
	}
	const char* cmd = std::getenv("DESKTOP_SESSION");
	if (!cmd) {
		cmd = "/bin/xinit";
	}
	::setenv("USER", username, TRUE);
	::setenv("LOGNAME", username, TRUE);
	::setenv("USERNAME", username, TRUE);
	::setenv("HOME", pw->pw_dir, TRUE);
	::setenv("SHELL", pw->pw_shell, TRUE);
	::setenv("PWD", pwd, TRUE);
	UNISTDX_CHECK(::initgroups(username, pw->pw_gid));
	UNISTDX_CHECK(::setregid(pw->pw_gid, pw->pw_gid));
	UNISTDX_CHECK(::setreuid(pw->pw_uid, pw->pw_uid));
	const char* argv[] = {
		pw->pw_shell, "-c", cmd, 0
	};
	::execve(argv[0], const_cast<char *const *>(argv), environ);
//	sys::this_process::execute(pw->pw_shell, "-c", cmd);
	std::exit(1);
}

void
log_in(GtkButton* btn, gpointer) {
	gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
	const char* username = gtk_entry_get_text(GTK_ENTRY(loginEntry));
	try {
		do_pam(username, converse_log_in);
		std::clog << "ok" << std::endl;
		fork_exec(username);
	} catch (const std::exception& err) {
		std::clog << "error: " << err.what() << std::endl;
		GtkWidget* dialog = gtk_message_dialog_new(
			GTK_WINDOW(window),
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			"%s",
			err.what()
		);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(btn), TRUE);
}

GtkWidget*
new_login_form() {
	// widgets
	GtkWidget* loginLabel = gtk_label_new(ggg::native("Login").data());
	gtk_label_set_xalign(GTK_LABEL(loginLabel), 1.f);
	loginEntry = gtk_entry_new();
	GtkWidget* passwordLabel = gtk_label_new(ggg::native("Password").data());
	gtk_label_set_xalign(GTK_LABEL(passwordLabel), 1.f);
	passwordEntry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(passwordEntry), FALSE);
	GtkWidget* logInButton = gtk_button_new_with_label(ggg::native("Log in").data());
	// layout
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
	gtk_grid_attach(GTK_GRID(grid), loginLabel, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), loginEntry, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), passwordLabel, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), passwordEntry, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), logInButton, 1, 2, 1, 1);
	// signals
	g_signal_connect(logInButton, "clicked", G_CALLBACK(log_in), nullptr);
	return grid;
}

void
activate_register_form(
	GtkNotebook* notebook,
	GtkWidget* page,
	guint page_num,
	gpointer user_data
) {
	if (page_num == 1 && !pam_thread.joinable()) {
		pam_thread = std::thread([&] () {
			load_form(username);
			do_pam(username, converse);
		});
	}
}

void
terminate_conversation() {
	state = Conversation_state::Quit;
	window = nullptr;
	pam_cv.notify_one();
}

void
login_form_app(GtkApplication* app, gpointer) {
	// window
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "GGG");
	gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	// widgets
	GtkWidget* loginPageGrid = new_login_form();
	GtkWidget* loginPageLabel = gtk_label_new(ggg::native("Log in").data());
	registerPageGrid = gtk_grid_new();
	GtkWidget* registerPageLabel = gtk_label_new(ggg::native("Register").data());
	// layout
	GtkWidget* notebook = gtk_notebook_new();
	gtk_notebook_append_page(
		GTK_NOTEBOOK(notebook),
		loginPageGrid,
		loginPageLabel
	);
	gtk_notebook_append_page(
		GTK_NOTEBOOK(notebook),
		registerPageGrid,
		registerPageLabel
	);
	gtk_container_add(GTK_CONTAINER(window), notebook);
	// signals
	g_signal_connect(
		notebook,
		"switch-page",
		G_CALLBACK(activate_register_form),
		nullptr
	);
	g_signal_connect(
		window,
		"destroy",
		G_CALLBACK(terminate_conversation),
		nullptr
	);
	gtk_widget_show_all(window);
}

int
main(int argc, char* argv[]) {
	int ret = EXIT_SUCCESS;
	try {
		parse_arguments(argc, argv);
		username = argv[::optind];
		gtk_init(nullptr, nullptr);
		if (form_type == Form_type::Login) {
			GtkApplication* app = gtk_application_new(
				"com.igankevich.ggg",
				G_APPLICATION_FLAGS_NONE
			);
			g_signal_connect(app, "activate", G_CALLBACK(login_form_app), NULL);
			ret = g_application_run(G_APPLICATION(app), 0, nullptr);
			g_object_unref(app);
		} else {
			throw std::runtime_error("please, use -l option");
		}
	} catch (const std::exception& err) {
		ret = EXIT_FAILURE;
		std::cerr << err.what() << std::endl;
	}
	if (pam_thread.joinable()) {
		pam_thread.join();
	}
	return ret;
}
