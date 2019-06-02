#include "add_entity.hh"

#include <iomanip>
#include <iostream>
#include <set>
#include <unistd.h>
#include <vector>

#include <unistdx/ipc/identity>

#include <ggg/cli/align_columns.hh>
#include <ggg/cli/editor.hh>
#include <ggg/cli/guile_traits.hh>
#include <ggg/cli/object_traits.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/cli/tmpfile.hh>
#include <ggg/config.hh>
#include <ggg/core/native.hh>

void
ggg::Add_entity
::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "t:f:e:")) != -1) {
		switch (opt) {
			case 't': std::string(::optarg) >> this->_type; break;
			case 'f': this->_filename = ::optarg; break;
			case 'e': this->_expression = ::optarg; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
	if (this->_args.empty() && this->_filename.empty() && this->_expression.empty()) {
		throw std::invalid_argument("please, specify entity names, filename or Guile expression");
	}
	remove_duplicate_arguments();
}

void
ggg::Add_entity
::execute() {
	scm_init_guile();
	scm_c_primitive_load(GGG_GUILE_ROOT "/types.scm");
	Database db;
	switch (this->_type) {
		case Entity_type::Entity:
			db.open(Database::File::All, Database::Flag::Read_write);
			this->do_execute<entity>(db);
			break;
		case Entity_type::Account:
			db.open(Database::File::Accounts, Database::Flag::Read_write);
			this->do_execute<account>(db);
			break;
		case Entity_type::Machine:
			throw std::invalid_argument("not implemented");
	}
}

template <class T>
void
ggg::Add_entity
::do_execute(Database& db) {
	if (!this->_filename.empty() || !this->_expression.empty()) {
		if (!this->_filename.empty()) {
			sys::tmpfile tmp;
			tmp.out().imbue(std::locale::classic());
			if (this->_filename == "-") {
				tmp.out() << std::cin.rdbuf();
			} else {
				tmp.out() << file_to_string(this->_filename);
			}
			tmp.out().flush();
			this->insert<T>(db, file_to_string(tmp.filename()));
		}
		if (!this->_expression.empty()) {
			this->insert<T>(db, this->_expression);
		}
	} else {
		this->add_interactive<T>(db);
	}
}

template <class T>
void
ggg::Add_entity
::add_interactive(Database& db) {
	using traits_type = Guile_traits<entity>;
	bool success = true;
	for (const std::string& ent : this->args()) {
		if (db.contains(ent.data())) {
			success = false;
			std::stringstream tmp;
			tmp << ent;
			native_message(std::cerr, "Entity _ already exists.", tmp.str());
		}
	}
	if (!success) { throw quiet_error(); }
	do {
		sys::tmpfile tmp(".scm");
		tmp.out().imbue(std::locale::classic());
		traits_type::to_guile(tmp.out(), traits_type::generate(args()));
		tmp.out().flush();
		edit_file_or_throw(tmp.filename());
		try {
			this->insert<T>(db, file_to_string(tmp.filename()));
		} catch (const std::exception& err) {
			success = false;
			native_message(std::cerr, "_", err.what());
			native_message(std::cerr, "Press any key to continue...");
			std::cin.get();
		}
	} while (!success);
}

template <class T>
void
ggg::Add_entity
::insert(Database& db, std::string guile) {
	using guile_traits_type = Guile_traits<T>;
	using traits_type = Object_traits<T>;
	auto ents = guile_traits_type::from_guile(guile);
	check_duplicates(ents, traits_type::eq);
	Transaction tr(db);
	for (const auto& ent : ents) {
		db.insert(ent);
		if (type() == Entity_type::Entity) {
			db.insert(account(ent.name().data()));
		}
	}
	tr.commit();
}

void
ggg::Add_entity
::print_usage() {
	const int w = 20;
	std::cout
		<< "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-t TYPE] [NAME...]\n"
		<< std::setw(w) << std::left << "  -t TYPE"
		<< "entity type (account, entity, machine)\n"
		<< std::setw(w) << std::left << "  -f FILE"
		<< "file to read entities from\n";
}

template void ggg::Add_entity::do_execute<ggg::entity>(Database&);
template void ggg::Add_entity::do_execute<ggg::account>(Database&);
template void ggg::Add_entity::add_interactive<ggg::entity>(Database&);
template void ggg::Add_entity::add_interactive<ggg::account>(Database&);
template void ggg::Add_entity::insert<ggg::entity>(Database&, std::string);
template void ggg::Add_entity::insert<ggg::account>(Database&, std::string);
