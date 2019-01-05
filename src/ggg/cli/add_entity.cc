#include "add_entity.hh"

#include <iomanip>
#include <iostream>
#include <set>
#include <unistd.h>
#include <vector>

#include <unistdx/ipc/identity>

#include "align_columns.hh"
#include "editor.hh"
#include "object_traits.hh"
#include "quiet_error.hh"
#include "tmpfile.hh"
#include <ggg/config.hh>
#include <ggg/core/native.hh>

namespace {

	const char* default_home = "/home";
	const char* default_shell = "/bin/sh";

}

void
ggg::Add_entity
::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "qf:d:")) != -1) {
		switch (opt) {
		case 'q': this->_verbose = false; break;
		case 'f':
			this->_path = ::optarg;
			this->_type = Type::File;
			break;
		case 'd':
			this->_path = ::optarg;
			this->_type = Type::Directory;
			break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace(argv[i]);
	}
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Add_entity
::execute() {
	Database db(Database::File::Entities, Database::Flag::Read_write);
	db.attach(Database::File::Accounts, Database::Flag::Read_write);
	if (this->is_batch()) {
		this->add_batch(db);
	} else {
		this->add_interactive(db);
	}
}

void
ggg::Add_entity
::add_interactive(Database& db) {
	bool success = true;
	for (const std::string& ent : this->args()) {
		if (db.contains(ent.data())) {
			success = false;
			std::stringstream tmp;
			tmp << ent;
			native_message(std::cerr, "Entity _ already exists.", tmp.str());
		}
	}
	if (!success) {
		throw quiet_error();
	}
	while (!this->_args.empty()) {
		sys::tmpfile tmp;
		tmp.out().imbue(std::locale::classic());
		this->generate_entities(db, tmp.out());
		edit_file_or_throw(tmp.filename());
		try {
			this->add_entities(db, sys::path(tmp.filename()));
		} catch (const quiet_error& err) {
			// ignore quiet error
		}
		if (!this->_args.empty()) {
			native_message(std::clog, "Press any key to continue...");
			std::cin.get();
		}
	}
}

void
ggg::Add_entity
::add_batch(Database& db) {
	sys::tmpfile tmp;
	tmp.out().imbue(std::locale::classic());
	tmp.out() << std::cin.rdbuf();
	tmp.out().flush();
	this->add_entities(db, sys::path(tmp.filename()));
}

void
ggg::Add_entity
::generate_entities(Database& db, std::ostream& out) {
	std::set<entity> cnt;
	for (const auto& name : this->args()) {
		entity tmp(name.data());
		tmp.home(sys::path(default_home, name));
		tmp.shell(default_shell);
		cnt.emplace(std::move(tmp));
	}
	align_columns(cnt, out, entity::delimiter, entity::delimiter, false);
	out.flush();
}

void
ggg::Add_entity
::add_entities(Database& db, sys::path filename) {
	typedef Object_traits<entity> traits_type;
	std::vector<entity> ents;
	read_objects<entity>(
		filename,
		std::back_inserter(ents),
		native("Unable to read entities.")
	);
	check_duplicates(ents, traits_type::eq);
	int nerrors = 0;
	for (entity& ent : ents) {
		try {
			Transaction tr(db);
			db.insert(ent);
			db.insert(account(ent.name().data()));
			this->_args.erase(traits_type::name(ent));
		} catch (const std::exception& err) {
			++nerrors;
			native_sentence(std::cerr, "Error adding _. ", ent);
			error_message(std::cerr, err);
		}
	}
	if (nerrors > 0) {
		throw quiet_error();
	}
}

void
ggg::Add_entity
::print_usage() {
	const int w = 20;
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-q] [-f FILE] [-d DIR] [ENTITY...]\n"
	          << std::setw(w) << std::left << "  -q" << "quiet\n"
	          << std::setw(w) << std::left << "  -f FILE" <<
	"file to add entities to\n"
	          << std::setw(w) << std::left << "  -d DIR" <<
	"directory to add entities to\n";
}
