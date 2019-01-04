#include <iostream>
#include <regex>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/lock.hh>
#include <ggg/core/native.hh>
#include <ggg/ctl/form.hh>
#include <ggg/ctl/password.hh>
#include <ggg/sec/echo_guard.hh>
#include <ggg/sec/secure_string.hh>

#include <unistdx/ipc/identity>

namespace ggg {

	const int max_iterations = 100;

	std::string
	input_field(const form_field& ff, bits::wcvt_type& cv) {
		std::wregex expr(cv.from_bytes(ff.regex()));
		std::string value;
		bool valid = false;
		int i = 0;
		do {
			value.clear();
			std::cout << ff.name() << ": " << std::flush;
			std::getline(std::cin, value, '\n');
			if (std::cin) {
				valid = std::regex_match(cv.from_bytes(value), expr);
			} else {
				std::cin.clear();
			}
		} while (!valid && ++i < max_iterations);
		if (!valid) {
			throw std::runtime_error("reached maximum number of attempts");
		}
		return value;
	}

	std::string
	input_password(
		const form_field& ff,
		bits::wcvt_type& cv,
		double min_entropy
	) {
		std::wstring name = cv.from_bytes(ff.name());
		std::wregex expr(cv.from_bytes(ff.regex()));
		std::string value;
		std::string rep;
		bool success = false;
		int i = 0;
		do {
			value.clear();
			rep.clear();
			{
				std::cout << ff.name() << ": " << std::flush;
				echo_guard g(STDIN_FILENO);
				std::getline(std::cin, value, '\n');
			}
			{
				std::cout << std::endl;
				std::cout << ff.name()
				          << " (" << native("repeat") << "): "
				          << std::flush;
				echo_guard g(STDIN_FILENO);
				std::getline(std::cin, rep, '\n');
			}
			bool valid = false;
			if (std::cin) {
				try {
					ggg::validate_password(value.data(), min_entropy);
					valid = std::regex_match(cv.from_bytes(value), expr);
				} catch (const std::exception& err) {
					std::cerr << std::endl;
					error_message(std::cerr, err);
					valid = false;
				}
			} else {
				std::cin.clear();
			}
			success = valid && rep == value;
			if (!success) {
				native_message(std::cerr, "Passwords do not match.");
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
				std::string value;
				if (ff.type() == field_type::password) {
					value = input_password(ff, cv, f.min_entropy());
				} else {
					value = input_field(ff, cv);
				}
				responses.emplace_back(value);
			}
		}
		std::tie(ent, acc) = f.make_entity_and_account(responses);
		return std::make_tuple(ent, acc);
	}

	void
	register_user(form& f) {
		const int max_iterations = 7;
		bool success;
		int i = 0;
		do {
			try {
				entity ent;
				account acc;
				std::tie(ent, acc) = input_entity_and_account(f);
				Database db(GGG_DATABASE_PATH, false);
				db.insert(ent);
				db.update(acc);
				success = true;
			} catch (const std::system_error& err) {
				std::clog << "err.what()=" << err.what() << std::endl;
				success = false;
				if (std::errc(err.code().value()) ==
				    std::errc::permission_denied) {
					throw;
				}
			} catch (const std::exception& err) {
				success = false;
				std::cerr << std::endl;
				error_message(std::cerr, err);
			}
		} while (!success && ++i < max_iterations);
		if (!success) {
			throw std::runtime_error("reached maximum number of attempts");
		}
		std::cout << std::endl;
		native_message(std::cout, "Registered successfully!");
	}

}

int
main() {
	using namespace ggg;
	int ret = 0;
	try {
		form f;
		f.set_type(form_type::console);
		{
			Database db(GGG_DATABASE_PATH);
			auto name = db.find_name(sys::this_process::user());
			if (name.empty()) {
				throw std::invalid_argument("unable to find form");
			}
			f.open(name.data());
		}
		init_locale(f.locale());
		register_user(f);
	} catch (const std::exception& err) {
		ret = 1;
		error_message(std::cerr, err);
	}
	if (::isatty(STDIN_FILENO)) {
		native_message(std::cout, "Press any key to continue...");
		std::cin.get();
	}
	return ret;
}
