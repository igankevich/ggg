#include <unistd.h>
#include <iostream>
#include <stdexcept>

#include "config.hh"
#include "all_commands.hh"
#include "quiet_error.hh"

void
show_error(const char* what) {
	std::cerr << "error: " << what << std::endl;
}

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
		show_error("memory allocation error");
	} catch (const std::invalid_argument& err) {
		show_error(err.what());
		if (cmd && !parse_success) {
			cmd->print_usage();
		}
	} catch (const std::exception& err) {
		show_error(err.what());
	}
	return ret;
}
