#include <grp.h>
#include <pwd.h>
#include <sys/types.h>

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
#include <vector>

#include <ggg/config.hh>
#include <ggg/core/hierarchy.hh>
#include <ggg/core/lock.hh>
#include <ggg/core/native.hh>
#include <ggg/ctl/account_ctl.hh>
#include <ggg/ctl/form.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/ctl/password.hh>
#include <ggg/pam/conversation.hh>
#include <ggg/pam/pam_call.hh>
#include <ggg/pam/pam_handle.hh>

#include <unistdx/base/check>
#include <unistdx/base/log_message>
#include <unistdx/ipc/identity>
#include <unistdx/util/backtrace>

std::string form_user;
ggg::form::container_type fields;
ggg::form form;
sys::path origin;
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
bool all_valid = false;
volatile bool form_loaded = false;


bool
regex_match(const char* string, const std::string& expr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
	std::wregex reg(cv.from_bytes(expr));
	std::wstring value(cv.from_bytes(string));
	return std::regex_match(value, reg);
}

gboolean
show_message_box(const char* msg, GtkMessageType type) {
	GtkWidget* dialog = gtk_message_dialog_new(
		GTK_WINDOW(window),
		GTK_DIALOG_MODAL,
		type,
		GTK_BUTTONS_OK,
		"%s",
		msg
	                    );
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return FALSE;
}

void
register_user(GtkButton*, gpointer) {
	if (!all_valid) return;
	// disable the button
	gtk_widget_set_sensitive(GTK_WIDGET(registerButton), FALSE);
	gtk_button_set_label(
		GTK_BUTTON(registerButton),
		ggg::native(
			"Please wait..."
		).data()
	);
	try {
		using namespace ggg;
		entity ent;
		account acc;
		std::tie(ent, acc) = ::form.make_entity_and_account(all_values);
		file_lock lock;
		GGG g(GGG_ENT_ROOT, true);
		lock.unlock();
		bool has_uid = ent.has_id();
		bool has_gid = ent.has_gid();
		if (!has_uid || !has_gid) {
			sys::uid_type id = g.next_uid();
			if (!has_uid) {
				ent.set_uid(id);
			}
			if (!has_gid) {
				ent.set_gid(id);
			}
		}
		{
			file_lock lock(true);
			acc.origin(origin);
			g.add(ent, acc);
		}
		show_message_box("Registered successfully!", GTK_MESSAGE_INFO);
		for (GtkWidget* entry : all_entries) {
			gtk_entry_set_text(GTK_ENTRY(entry), "");
		}
	} catch (const std::exception& err) {
		show_message_box(err.what(), GTK_MESSAGE_ERROR);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(registerButton), TRUE);
	gtk_button_set_label(
		GTK_BUTTON(registerButton),
		ggg::native("Register").data()
	);
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

bool
validate_password() {
	auto result = std::find_if(
		fields.begin(),
		fields.end(),
		[&] (const ggg::form_field& rhs) {
		    return rhs.type() == ggg::field_type::set_secure &&
		    rhs.target() == "account.password";
		}
	              );
	bool valid = true;
	const char* errorText = "";
	if (result != fields.end()) {
		const ggg::form_field& ff = *result;
		ggg::form_field::id_type id = 0;
		std::stringstream str(ff.regex());
		char ch = str.get();
		if (ch != '$') {
			str.putback(ch);
		}
		str >> id;
		auto it = values.find(ggg::form_field(id));
		if (it != values.end()) {
			const char* new_password = it->second;
			ggg::password_match match;
			if (match.find(new_password) &&
			    match.entropy() < form.min_entropy()) {
				valid = false;
			}
		}
		errorText = (valid) ? "" : "Weak password";
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
	valid &= validate_password();
	all_valid = valid;
	gtk_widget_set_sensitive(registerButton, valid);
}

template <class T>
T*
allocate(size_t n) {
	return reinterpret_cast<T*>(std::malloc(sizeof(T)*n));
}

std::vector<GtkWidget*>
new_widget(const ggg::form_field& ff) {
	std::vector<GtkWidget*> ret;
	if (ff.is_input()) {
		GtkWidget* label = gtk_label_new(ff.name().data());
		gtk_label_set_xalign(GTK_LABEL(label), 1.f);
		GtkWidget* entry = gtk_entry_new();
		if (ff.type() == ggg::field_type::password) {
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
	} else {
		GtkWidget* label = gtk_label_new(ff.name().data());
		ret.push_back(label);
	}
	return ret;
}

GtkWidget*
new_widget_form() {
	all_entries.clear();
	all_error_labels.clear();
	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
	const int n = fields.size();
	int num_input_fields = 0;
	for (int i=0; i<n; ++i) {
		const ggg::form_field& ff = fields[i];
		if (ff.is_input()) {
			std::vector<GtkWidget*> widgets = new_widget(ff);
			const int nwidgets = widgets.size();
			for (int j=0; j<nwidgets; ++j) {
				gtk_grid_attach(GTK_GRID(grid), widgets[j], j, i, 1, 1);
			}
			++num_input_fields;
		}
	}
	// widgets
	registerButton = gtk_button_new_with_label(ggg::native("Register").data());
	gtk_widget_set_sensitive(registerButton, FALSE);
	loginErrorLabel = gtk_label_new("");
	// layout
	gtk_grid_attach(GTK_GRID(grid), registerButton, 1, num_input_fields, 1, 1);
	gtk_grid_attach(
		GTK_GRID(grid),
		loginErrorLabel,
		0,
		num_input_fields+1,
		3,
		1
	);
	return grid;
}

gboolean
show_registration_form() {
	GtkWidget* newRegisterForm = new_widget_form();
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

int
converse_log_in(
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

void
load_form() {
	form.clear();
	form.set_type(ggg::form_type::graphical);
	ggg::file_lock lock;
	ggg::Hierarchy h(GGG_ENT_ROOT);
	auto result = h.find_by_name(form_user.data());
	if (result == h.end()) {
		throw std::invalid_argument("unable to find form");
	}
	form.open(result->name().data());
	origin = sys::path(GGG_ROOT, "acc", result->name());
	fields = form.fields();
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
chdir_error(const char* rhs, const char* error) {
	sys::log_message(
		"error",
		"failed to change directory to \"_\": _",
		rhs,
		error
	);
}

const char*
change_directory(const char* pwd) {
	try {
		UNISTDX_CHECK(::chdir(pwd));
	} catch (const std::exception& err) {
		chdir_error(pwd, err.what());
		try {
			pwd = "/";
			UNISTDX_CHECK(::chdir(pwd));
		} catch (const std::exception& err2) {
			chdir_error(pwd, err.what());
			throw;
		}
	}
	return pwd;
}

void
set_environment(const char* name, const char* value) {
	UNISTDX_CHECK(::setenv(name, value, TRUE))
}

void
launch_desktop(const char* username) {
	struct ::passwd* pw = ::getpwnam(username);
	if (!pw) {
		std::clog << "user " << username << " not found via NSS." << std::endl;
		std::exit(1);
	}
	const char* pwd = change_directory(pw->pw_dir);
	const char* cmd = std::getenv("DESKTOP_SESSION");
	if (!cmd) {
		cmd = "/bin/xinit";
	}
	set_environment("USER", username);
	set_environment("LOGNAME", username);
	set_environment("USERNAME", username);
	set_environment("HOME", pw->pw_dir);
	set_environment("SHELL", pw->pw_shell);
	set_environment("PWD", pwd);
	UNISTDX_CHECK(::initgroups(username, pw->pw_gid));
	UNISTDX_CHECK(::setregid(pw->pw_gid, pw->pw_gid));
	UNISTDX_CHECK(::setreuid(pw->pw_uid, pw->pw_uid));
	const char* argv[] = {
		pw->pw_shell, "-c", cmd, 0
	};
	UNISTDX_CHECK(::execve(argv[0], const_cast<char* const*>(argv), environ));
	std::exit(1);
}

void
log_in(GtkButton* btn, gpointer) {
	gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
	const char* username = gtk_entry_get_text(GTK_ENTRY(loginEntry));
	try {
		do_pam(username, converse_log_in);
		std::clog << "ok" << std::endl;
		launch_desktop(username);
	} catch (const std::exception& err) {
		sys::backtrace(STDERR_FILENO);
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
	GtkWidget* logInButton = gtk_button_new_with_label(
		ggg::native(
			"Log in"
		).data()
	                         );
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
	if (page_num == 1 && !form_loaded) {
		try {
			load_form();
			form_loaded = true;
			show_registration_form();
		} catch (const std::exception& err) {
			sys::log_message(
				"error",
				"failed to init registration form: _",
				err.what()
			);
		}
	}
}

void
terminate_conversation() {
	window = nullptr;
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
	GtkWidget* registerPageLabel = gtk_label_new(
		ggg::native(
			"Register"
		).data()
	                               );
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

void
parse_arguments(int argc, char* argv[]) {
	if (argc != 2) {
		throw std::invalid_argument(
				  "please, specify username as the first argument"
		);
	}
	form_user = argv[1];
}

int
main(int argc, char* argv[]) {
	int ret = EXIT_SUCCESS;
	try {
		parse_arguments(argc, argv);
		gtk_init(nullptr, nullptr);
		GtkApplication* app = gtk_application_new(
			"com.igankevich.ggg",
			G_APPLICATION_FLAGS_NONE
		                      );
		g_signal_connect(app, "activate", G_CALLBACK(login_form_app), NULL);
		ret = g_application_run(G_APPLICATION(app), 0, nullptr);
		g_object_unref(app);
	} catch (const std::exception& err) {
		ret = EXIT_FAILURE;
		std::cerr << err.what() << std::endl;
	}
	return ret;
}
