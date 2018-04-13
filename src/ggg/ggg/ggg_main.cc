#include <unistd.h>
#include <iostream>
#include <stdexcept>

#include <ggg/config.hh>
#include "all_commands.hh"
#include "quiet_error.hh"
#include <ggg/core/native.hh>

ggg::command_ptr
parse_command(int argc, char* argv[]) {
	ggg::command_ptr cmd = nullptr;
	if (argc >= 1 && argv[1]) {
		cmd = ggg::command_from_string(argv[1]);
		::optind = 2;
	}
	if (!cmd) {
		throw std::invalid_argument("bad command");
	}
	return cmd;
}

int main(int argc, char* argv[]) {
	ggg::init_locale();
	ggg::command_ptr cmd = nullptr;
	int ret = EXIT_FAILURE;
	bool parse_success = false;
	try {
		cmd = parse_command(argc, argv);
		cmd->parse_arguments(argc, argv);
		parse_success = true;
		cmd->execute();
		ret = EXIT_SUCCESS;
	} catch (const ggg::quiet_error& err) {
		// info has been already printed
	} catch (const std::bad_alloc& err) {
		ggg::native_message(std::wcerr, "memory allocation error");
	} catch (const std::invalid_argument& err) {
		ggg::error_message(std::wcerr, err);
		if (cmd && !parse_success) {
			cmd->print_usage();
		}
	} catch (const std::exception& err) {
		ggg::error_message(std::wcerr, err);
	}
	return ret;
}
