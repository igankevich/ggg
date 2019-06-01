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

namespace {

	const char* default_home = "/home";
	const char* default_shell = "/bin/sh";

}

void
ggg::Add_entity
::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "q")) != -1) {
		switch (opt) {
		case 'q': this->_verbose = false; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Add_entity
::execute() {
	scm_init_guile();
	scm_c_primitive_load(GGG_GUILE_ROOT "/types.scm");
	Database db(Database::File::All, Database::Flag::Read_write);
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
	do {
		sys::tmpfile tmp(".scm");
		tmp.out().imbue(std::locale::classic());
		this->generate_entities(db, tmp.out());
		edit_file_or_throw(tmp.filename());
		try {
			this->add_entities_guile(db, sys::path(tmp.filename()));
		} catch (const std::exception& err) {
			success = false;
			native_message(std::cerr, "_", err.what());
			native_message(std::cerr, "Press any key to continue...");
			std::cin.get();
		}
	} while (!success);
}

void
ggg::Add_entity
::add_batch(Database& db) {
	sys::tmpfile tmp;
	tmp.out().imbue(std::locale::classic());
	tmp.out() << std::cin.rdbuf();
	tmp.out().flush();
	this->add_entities(db, sys::path(tmp.filename()), entity_format::batch);
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
	using traits_type = Guile_traits<entity>;
	if (cnt.size() == 1) {
		out << traits_type::to_guile(*cnt.begin());
	} else {
		out << "(list ";
		auto first = cnt.begin();
		auto last = cnt.end();
		out << traits_type::to_guile(*first, 1);
		++first;
		while (first != last) {
			out << '\n' << traits_type::to_guile(*first, 1, true);
			++first;
		}
		out << ")\n";
	}
	out.flush();
}

void
ggg::Add_entity
::add_entities(
	Database& db,
	sys::path filename,
	entity_format format
) {
	typedef Object_traits<entity> traits_type;
	std::vector<entity> ents;
	read_objects<entity>(
		filename,
		format,
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
			tr.commit();
			auto result = std::find_if(
				args().begin(),
				args().end(),
				[&ent] (const std::string& s) {
					return s == traits_type::name(ent);
				}
			);
			if (result != args().end()) {
				this->_args.erase(result);
			}
		} catch (const std::exception& err) {
			++nerrors;
			native_sentence(std::cerr, "Error adding _. ", ent.name());
			error_message(std::cerr, err);
		}
	}
	if (nerrors > 0) {
		throw quiet_error();
	}
}

void
ggg::Add_entity
::add_entities_guile(Database& db, sys::path filename) {
	using guile_traits_type = Guile_traits<entity>;
	using traits_type = Object_traits<entity>;
	std::ifstream in;
	std::stringstream guile;
	try {
		in.exceptions(std::ios::failbit | std::ios::badbit);
		in.imbue(std::locale::classic());
		in.open(filename, std::ios_base::in);
		guile << in.rdbuf();
		in.close();
	} catch (...) {
		if (!in.eof()) {
			throw std::system_error(std::io_errc::stream,
				native("Unable to read entities."));
		}
	}
	auto ents = guile_traits_type::from_guile(guile.str());
	check_duplicates(ents, traits_type::eq);
	Transaction tr(db);
	for (const auto& ent : ents) {
		db.insert(ent);
		db.insert(account(ent.name().data()));
	}
	tr.commit();
}

void
ggg::Add_entity
::print_usage() {
	const int w = 20;
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-q] [ENTITY...]\n"
	          << std::setw(w) << std::left << "  -q" << "quiet\n";
}
