#include <ggg/cli/attach.hh>

#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Attach::execute()  {
	if (this->_args.size() != 2) {
		throw std::invalid_argument("please, specify exactly two names");
	}
	if (args().front() == args().back()) {
		throw std::invalid_argument("please, specify different names");
	}
	const auto& child = args().front();
	const auto& parent = args().back();
	Database db(Database::File::Entities, Database::Flag::Read_write);
	db.attach(child.data(), parent.data());
}

void
ggg::Attach::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " CHILD PARENT\n";
}


void
ggg::Detach::execute() {
	if (args().empty()) {
		throw std::invalid_argument("please, specify at least one name");
	}
	remove_duplicate_arguments();
	Database db(Database::File::Entities, Database::Flag::Read_write);
	for (const auto& child : args()) {
		db.detach(child.data());
	}
}

void
ggg::Detach::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " ENTITY...\n";
}



void
ggg::Tie::execute()  {
	if (this->_args.size() != 2) {
		throw std::invalid_argument("please, specify exactly two names");
	}
	if (args().front() == args().back()) {
		throw std::invalid_argument("please, specify different names");
	}
	const auto& child = args().front();
	const auto& parent = args().back();
	Database db(Database::File::Entities, Database::Flag::Read_write);
	db.tie(child.data(), parent.data());
}

void
ggg::Tie::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " CHILD PARENT\n";
}

void
ggg::Untie::execute() {
	remove_duplicate_arguments();
	auto n = args().size();
	if (n != 1 && n != 2) {
		throw std::invalid_argument("please, specify one or two names");
	}
	const auto& child = args().front();
	Database db(Database::File::Entities, Database::Flag::Read_write);
	if (n == 1) {
		db.untie(child.data());
	} else if (n == 2) {
		const auto& parent = args().back();
		db.untie(child.data(), parent.data());
	}
}

void
ggg::Untie::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " CHILD [PARENT]\n";
}
