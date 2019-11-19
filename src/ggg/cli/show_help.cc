#include "show_help.hh"

#include <iomanip>
#include <iostream>
#include <unistd.h>

#include <ggg/config.hh>

void
ggg::Show_help::parse_arguments(int argc, char* argv[])  {
	if (*this->prefix().data() != '-' && ::optind < argc) {
		const char* str = argv[::optind];
		if (str && str[0] != '-') {
			try {
				this->_ptr = command_from_string(str);
			} catch (...) {
			}
		}
	}
}

void
ggg::Show_help::execute()  {
	if (this->_ptr) {
		this->_ptr->print_usage();
	} else {
		print_usage();
	}
}

void
ggg::Show_help::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " COMMAND" << '\n';
	const int col1 = 25;
	std::cout
		<< "Database commands:\n"
		<< std::setw(col1) << std::right << "init,heal  "
		<< std::left << "initialise database and fix permissions\n"
		<< std::setw(col1) << std::right << "backup  "
		<< std::left << "backup database\n"
		<< "Entity commands:\n"
		<< std::setw(col1) << std::right << "rm,delete,remove  "
		<< std::left << "delete entity\n"
		<< std::setw(col1) << std::right << "add,insert,new  "
		<< std::left << "add entity\n"
		<< std::setw(col1) << std::right << "find,search  "
		<< std::left << "find entity by name\n"
		<< std::setw(col1) << std::right << "info,show  "
		<< std::left << "show entity information\n"
		<< std::setw(col1) << std::right << "groups,parents  "
		<< std::left << "list entity groups\n"
		<< std::setw(col1) << std::right << "members,children  "
		<< std::left << "list group members\n"
		<< std::setw(col1) << std::right << "attach  "
		<< std::left << "attach entity to the new hierarchy\n"
		<< std::setw(col1) << std::right << "detach  "
		<< std::left << "detach entity from the current hierarchy\n"
		<< std::setw(col1) << std::right << "tie  "
		<< std::left << "tie entities from different hierarchies\n"
		<< std::setw(col1) << std::right << "untie  "
		<< std::left << "untie entities from different hierarchies\n"
		<< "Account commands:\n"
		<< std::setw(col1) << std::right << "expire  "
		<< std::left << "permanently expire account\n"
		<< std::setw(col1) << std::right << "expired  "
		<< std::left << "list expired account\n"
		<< std::setw(col1) << std::right << "lock,suspend,deactivate  "
		<< std::left << "temporarily lock account\n"
		<< std::setw(col1) << std::right << "unlock,resume,activate  "
		<< std::left << "unlock account\n"
		<< std::setw(col1) << std::right << "locked,inactive  "
		<< std::left << "list temporarily locked accounts\n"
		<< std::setw(col1) << std::right << "expunge,purge  "
		<< std::left << "terminate all processes belonging to locked accounts\n"
		<< std::setw(col1) << std::right << "reset  "
		<< std::left << "reset password\n"
		<< "Machines commands:\n"
		<< std::setw(col1) << std::right << "machines,hosts  "
		<< std::left << "list all machines\n"
		<< "Message log commands:\n"
		<< std::setw(col1) << std::right << "messages,log  "
		<< std::left << "show log messages\n"
		<< std::setw(col1) << std::right << "rotate  "
		<< std::left << "remove old log messages\n"
		<< "Other commands:\n"
		<< std::setw(col1) << std::right << "guile,script  "
		<< std::left << "run Guile script that manipulates entities\n"
		<< std::setw(col1) << std::right << "dot,graphviz,graph  "
		<< std::left << "output entity graph in dot format\n"
		<< std::setw(col1) << std::right << "version  "
		<< std::left << "programme version\n"
		<< std::setw(col1) << std::right << "help  "
		<< std::left << "this help message\n"
		;
}

