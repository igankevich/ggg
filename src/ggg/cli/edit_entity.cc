#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <iterator>
#include <vector>
#include <cstdlib>

#include <unistdx/fs/path>
#include <unistdx/io/fdstream>

#include <ggg/cli/align_columns.hh>
#include <ggg/cli/edit_entity.hh>
#include <ggg/cli/editor.hh>
#include <ggg/cli/object_traits.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/cli/tmpfile.hh>
#include <ggg/config.hh>
#include <ggg/core/native.hh>

void
ggg::Edit_entity::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "qae")) != -1) {
		switch (opt) {
			case 'q': this->_verbose = false; break;
			case 'a': this->_type = Type::Account; break;
			case 'e': this->_type = Type::Entity; break;
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
ggg::Edit_entity::execute()  {
	Database db(GGG_ENTITIES_PATH, false);
	switch (this->_type) {
		case Type::Entity: this->edit_objects<entity>(db); break;
		case Type::Account: this->edit_objects<account>(db); break;
	}
}

template <class T>
void
ggg::Edit_entity::edit_objects(Database& db) {
	if (this->is_batch()) {
		this->edit_batch<T>(db);
	} else {
		this->edit_interactive<T>(db);
	}
}

template <class T>
void
ggg::Edit_entity::edit_interactive(Database& db) {
	while (!this->_args.empty()) {
		sys::tmpfile tmp;
		tmp.out().imbue(std::locale::classic());
		this->print_objects<T>(db, tmp.out());
		edit_file_or_throw(tmp.filename());
		this->update_objects<T>(db, tmp.filename());
		if (!this->_args.empty()) {
			native_message(std::clog, "Press any key to continue...");
			std::cin.get();
		}
	}
}

template <class T>
void
ggg::Edit_entity::edit_batch(Database& db) {
	sys::tmpfile tmp;
	tmp.out().imbue(std::locale::classic());
	tmp.out() << std::cin.rdbuf();
	tmp.out().flush();
	this->update_objects<T>(db, tmp.filename());
}

template <class T>
void
ggg::Edit_entity::print_objects(Database& db, std::ostream& out) {
	typedef Object_traits<T> traits_type;
	std::set<T> cnt;
	traits_type::find(db, this->args(), std::inserter(cnt, cnt.begin()));
	if (cnt.empty()) {
		throw std::runtime_error("not found");
	}
	align_columns(cnt, out, traits_type::delimiter());
	out.flush();
}

void
ggg::Edit_entity::print_usage() {
	const int w = 20;
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-qaed] [ENTITY...]\n"
		<< std::setw(w) << std::left << "  -q" << "quiet\n"
		<< std::setw(w) << std::left << "  -a ACCOUNT..." << "edit accounts\n"
		<< std::setw(w) << std::left << "  -e ENTITY..." << "edit entities\n"
		<< std::setw(w) << std::left << "  -d" << "edit directory hierarchy\n";
}

template <class T>
void
ggg::Edit_entity::update_objects(Database& db, const std::string& filename) {
	typedef Object_traits<T> traits_type;
	std::vector<T> ents;
	read_objects<T>(
		filename,
		std::back_inserter(ents),
		"unable to read entities"
	);
	check_duplicates(ents, traits_type::eq);
	int nerrors = 0;
	for (const T& ent : ents) {
		try {
			db.update(ent);
			this->_args.erase(traits_type::name(ent));
		} catch (const std::exception& err) {
			++nerrors;
			native_message(std::cerr, "error updating _: _", ent, native(err.what()));
		}
	}
	if (nerrors > 0) {
		throw quiet_error();
	}
}

template void
ggg::Edit_entity::print_objects<ggg::entity>(Database& db, std::ostream& out);
template void
ggg::Edit_entity::print_objects<ggg::account>(Database& db, std::ostream& out);
template void
ggg::Edit_entity::update_objects<ggg::entity>(Database& db, const std::string& filename);
template void
ggg::Edit_entity::update_objects<ggg::account>(Database& db, const std::string& filename);
template void
ggg::Edit_entity::edit_objects<ggg::entity>(Database& db);
template void
ggg::Edit_entity::edit_objects<ggg::account>(Database& db);
template void
ggg::Edit_entity::edit_batch<ggg::entity>(Database& db);
template void
ggg::Edit_entity::edit_batch<ggg::account>(Database& db);
template void
ggg::Edit_entity::edit_interactive<ggg::entity>(Database& db);
template void
ggg::Edit_entity::edit_interactive<ggg::account>(Database& db);
