#include "add_entity.hh"

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <vector>

#include "quiet_error.hh"
#include "align_columns.hh"
#include "tmpfile.hh"
#include "editor.hh"
#include "object_traits.hh"
#include "config.hh"

void
ggg::Add_entity::parse_arguments(int argc, char* argv[]) {
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
ggg::Add_entity::execute()  {
	GGG g(GGG_ROOT, this->verbose());
	bool success = true;
	for (const std::string& ent : this->args()) {
		if (g.contains(ent)) {
			success = false;
			std::cerr << "error: entity " << ent << " already exists" << std::endl;
		}
	}
	if (!success) {
		throw quiet_error();
	}
	while (!this->_args.empty()) {
		sys::tmpfile tmp;
		this->generate_entities(g, tmp.out());
		edit_file_or_throw(tmp.filename());
		this->add_entities(g, tmp.filename());
		if (!this->_args.empty()) {
			std::clog << "press any key to continue..." << std::endl;
			std::cin.get();
		}
	}
}

void
ggg::Add_entity::generate_entities(GGG& g, std::ostream& out) {
	std::vector<entity> cnt;
	for (const std::string& name : this->args()) {
		cnt.emplace_back(g.generate(name));
	}
	align_columns(cnt, out, entity::delimiter);
	out.flush();
}

void
ggg::Add_entity::add_entities(GGG& g, const std::string& filename) {
	typedef Object_traits<entity> traits_type;
	std::vector<entity> ents;
	read_objects<entity>(
		filename,
		std::back_inserter(ents),
		"unable to read entities"
	);
	check_duplicates(ents, traits_type::eq);
	for (const entity& ent : ents) {
		const std::string& fname = this->get_filename(ent);
		try {
			g.add(ent, fname);
			this->_args.erase(traits_type::name(ent));
		} catch (const std::exception& err) {
			std::cerr << "error adding ";
			traits_type::print(std::cerr, ent);
			std::cerr << ": " << err.what() << std::endl;
		}
	}
}

std::string
ggg::Add_entity::get_filename(const entity& ent) const {
	std::string filename;
	if (this->_type == Type::Directory) {
		if (this->_path.empty()) {
			filename = ent.name();
		} else {
			filename.append(this->_path);
			filename.push_back(sys::path::separator);
			filename.append(ent.name());
		}
	} else {
		filename = this->_path;
	}
	return filename;
}

void
ggg::Add_entity::print_usage() {
	const int w = 20;
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] [-f FILE] [-d DIR] [ENTITY...]\n"
		<< std::setw(w) << std::left << "  -q" << "quiet\n"
		<< std::setw(w) << std::left << "  -f FILE" << "file to add entities to\n"
		<< std::setw(w) << std::left << "  -d DIR" << "directory to add entities to\n";
}

