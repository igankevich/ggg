#include "edit_entity.hh"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <set>
#include <iterator>
#include <array>
#include <vector>
#include <cstdlib>

#include <unistdx/fs/path>
#include <unistdx/io/fdstream>

#include "config.hh"
#include "ctl/ggg.hh"
#include "editor.hh"
#include "tmpfile.hh"
#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Edit_entity::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "qaed")) != -1) {
		switch (opt) {
			case 'q': this->_verbose = false; break;
			case 'a': this->_type = Type::Account; break;
			case 'e': this->_type = Type::Entity; break;
			case 'd': this->_type = Type::Directory; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace(argv[i]);
	}
	if (this->_type == Type::Directory) {
		if (!this->_args.empty()) {
			throw std::invalid_argument(
				"arguments are not allowed when editing directory"
			);
		}
	} else {
		if (this->_args.empty()) {
			throw std::invalid_argument("please, specify entity names");
		}
	}
}

void
ggg::Edit_entity::execute()  {
	GGG g(GGG_ROOT, this->verbose());
	switch (this->_type) {
		case Type::Entity: this->edit_objects<entity>(g); break;
		case Type::Account: this->edit_objects<account>(g); break;
		case Type::Directory: this->edit_directory(); break;
	}
}

template <class T>
void
ggg::Edit_entity::edit_objects(GGG& g) {
	while (!this->_args.empty()) {
		sys::tmpfile tmp;
		this->print_objects<T>(g, tmp.out());
		edit_file_or_throw(tmp.filename());
		this->update_objects<T>(g, tmp.filename());
		if (!this->_args.empty()) {
			std::clog << "press any key to continue..." << std::endl;
			std::cin.get();
		}
	}
}

template <class T>
void
ggg::Edit_entity::print_objects(GGG& g, std::ostream& out) {
	typedef Object_traits<T> traits_type;
	std::set<T> cnt;
	traits_type::find(g, this->args(), std::inserter(cnt, cnt.begin()));
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
ggg::Edit_entity::update_objects(GGG& g, const std::string& filename) {
	typedef Object_traits<T> traits_type;
	std::vector<T> ents;
	read_objects<T>(
		filename,
		std::back_inserter(ents),
		"unable to read entities"
	);
	check_duplicates(ents, traits_type::eq);
	for (const T& ent : ents) {
		try {
			g.update(ent);
			this->_args.erase(traits_type::name(ent));
		} catch (const std::exception& err) {
			std::cerr << "error updating ";
			traits_type::print(std::cerr, ent);
			std::cerr << ": " << err.what() << std::endl;
		}
	}
}

void
ggg::Edit_entity::edit_directory() {
	sys::proc_status status;
	bool success;
	do {
		status = ::ggg::edit_directory(sys::path(GGG_ROOT));
		success = status.exited() && status.exit_code() == 0;
		if (!success) {
			std::cerr << "bad exit code from editor" << std::endl;
			std::clog << "press any key to continue..." << std::endl;
			std::cin.get();
		}
	} while (!success);
}

template void
ggg::Edit_entity::print_objects<ggg::entity>(GGG& g, std::ostream& out);
template void
ggg::Edit_entity::print_objects<ggg::account>(GGG& g, std::ostream& out);
template void
ggg::Edit_entity::update_objects<ggg::entity>(GGG& g, const std::string& filename);
template void
ggg::Edit_entity::update_objects<ggg::account>(GGG& g, const std::string& filename);
template void
ggg::Edit_entity::edit_objects<ggg::entity>(GGG& g);
template void
ggg::Edit_entity::edit_objects<ggg::account>(GGG& g);
