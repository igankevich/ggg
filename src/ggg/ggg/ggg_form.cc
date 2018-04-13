#include <iostream>
#include <regex>

#include <ggg/config.hh>
#include <ggg/core/hierarchy.hh>
#include <ggg/core/lock.hh>
#include <ggg/core/native.hh>
#include <ggg/ctl/form.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/ctl/password.hh>
#include <ggg/sec/echo_guard.hh>
#include <ggg/sec/secure_string.hh>

#include <unistdx/ipc/identity>

namespace ggg {

	const int max_iterations = 100;

	std::wstring
	input_field(const form_field& ff, bits::wcvt_type& cv) {
		std::wstring name = cv.from_bytes(ff.name());
		std::wregex expr(cv.from_bytes(ff.regex()));
		std::wstring value;
		bool valid = false;
		int i = 0;
		do {
			value.clear();
			std::wcout << name << L": " << std::flush;
			std::getline(std::wcin, value, L'\n');
			if (std::wcin) {
				valid = std::regex_match(value, expr);
			} else {
				std::wcin.clear();
			}
		} while (!valid && ++i < max_iterations);
		if (!valid) {
			throw std::runtime_error("reached maximum number of attempts");
		}
		return value;
	}

	std::wstring
	input_password(
		const form_field& ff,
		bits::wcvt_type& cv,
		double min_entropy
	) {
		std::wstring name = cv.from_bytes(ff.name());
		std::wregex expr(cv.from_bytes(ff.regex()));
		std::wstring value;
		std::wstring rep;
		bool success = false;
		int i = 0;
		do {
			value.clear();
			rep.clear();
			{
				std::wcout << name << L": " << std::flush;
				echo_guard g(STDIN_FILENO);
				std::getline(std::wcin, value, L'\n');
			}
			{
				std::wcout << std::endl;
				std::wcout << name
				           << L" (" << wnative("repeat", cv) << L"): "
				           << std::flush;
				echo_guard g(STDIN_FILENO);
				std::getline(std::wcin, rep, L'\n');
			}
			bool valid = false;
			if (std::wcin) {
				std::string p = cv.to_bytes(value);
				try {
					ggg::validate_password(p.data(), min_entropy);
					valid = std::regex_match(value, expr);
				} catch (const std::exception& err) {
					std::wcerr << std::endl << err.what() << std::endl;
					valid = false;
				}
			} else {
				std::wcin.clear();
			}
			success = valid && rep == value;
			if (!success) {
				native_message(std::wcerr, "Passwords do not match.");
			}
		} while (!success && ++i < max_iterations);
		if (!success) {
			throw std::runtime_error("reached maximum number of attempts");
		}
		return value;
	}

	std::tuple<entity,account>
	input_entity_and_account(form& f) {
		entity ent;
		account acc;
		std::vector<std::string> responses;
		bits::wcvt_type cv;
		for (const form_field& ff : f.fields()) {
			if (ff.is_input()) {
				std::wstring value;
				if (ff.type() == field_type::password) {
					value = input_password(ff, cv, f.min_entropy());
				} else {
					value = input_field(ff, cv);
				}
				responses.emplace_back(cv.to_bytes(value));
			}
		}
		std::tie(ent, acc) = f.make_entity_and_account(responses);
		return std::make_tuple(ent, acc);
	}

	void
	register_user(form& f, sys::path origin) {
		const bool debug = false;
		const int max_iterations = 7;
		bool success;
		int i = 0;
		do {
			try {
				entity ent;
				account acc;
				std::tie(ent, acc) = input_entity_and_account(f);
				file_lock lock;
				GGG g(GGG_ENT_ROOT, debug);
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
				success = true;
			} catch (const std::system_error& err) {
				success = false;
				if (std::errc(err.code().value()) ==
				    std::errc::permission_denied) {
					throw;
				}
			} catch (const std::exception& err) {
				success = false;
				std::wcerr << std::endl << err.what() << std::endl;
			}
		} while (!success && ++i < max_iterations);
		if (!success) {
			throw std::runtime_error("reached maximum number of attempts");
		}
		std::wcout << std::endl;
		native_message(std::wcout, "Registered successfully!");
	}

}

int
main() {
	int ret = 0;
	try {
		ggg::form f;
		f.set_type(ggg::form_type::console);
		sys::path origin;
		{
			ggg::file_lock lock;
			ggg::Hierarchy h(GGG_ENT_ROOT);
			auto result = h.find_by_uid(sys::this_process::user());
			if (result == h.end()) {
				throw std::invalid_argument("unable to find form");
			}
			f.open(result->name().data());
			origin = sys::path(GGG_ROOT, "acc", result->name(), "shadow");
		}
		ggg::init_locale(f.locale());
		ggg::register_user(f, origin);
	} catch (const std::exception& err) {
		ret = 1;
		std::wcerr << err.what() << std::endl;
	}
	if (::isatty(STDIN_FILENO)) {
		ggg::native_message(std::wcout, "Press any key to continue...");
		std::wcin.get();
	}
	return ret;
}
