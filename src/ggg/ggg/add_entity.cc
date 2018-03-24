#include "add_entity.hh"

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <vector>

#include <unistdx/ipc/identity>

#include "quiet_error.hh"
#include "align_columns.hh"
#include "tmpfile.hh"
#include "editor.hh"
#include "object_traits.hh"
#include <ggg/config.hh>
#include <ggg/core/native.hh>
#include <ggg/core/lock.hh>

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
	file_lock lock;
	GGG g(GGG_ENT_ROOT, this->verbose());
	lock.unlock();
	bool success = true;
	for (const std::string& ent : this->args()) {
		if (g.contains(ent)) {
			success = false;
			native_message(std::cerr, "error: entity _ already exists\n", ent);
		}
	}
	if (!success) {
		throw quiet_error();
	}
	while (!this->_args.empty()) {
		sys::tmpfile tmp;
		tmp.out().imbue(std::locale::classic());
		this->generate_entities(g, tmp.out());
		edit_file_or_throw(tmp.filename());
		this->add_entities(g, sys::path(tmp.filename()));
		if (!this->_args.empty()) {
			std::clog << "press any key to continue..." << std::endl;
			std::cin.get();
		}
	}
}

void
ggg::Add_entity::generate_entities(GGG& g, std::ostream& out) {
	file_lock lock;
	std::set<entity> cnt = g.generate(this->args());
	align_columns(cnt, out, entity::delimiter, entity::delimiter, false);
	out.flush();
}

void
ggg::Add_entity::add_entities(GGG& g, sys::path filename) {
	file_lock lock(true);
	const sys::uid_type uid = sys::this_process::user();
	sys::path acc_origin = g.account_origin(uid);
	typedef Object_traits<entity> traits_type;
	std::vector<entity> ents;
	read_objects<entity>(
		filename,
		std::back_inserter(ents),
		"unable to read entities"
	);
	check_duplicates(ents, traits_type::eq);
	std::clog.imbue(std::locale::classic());
	for (entity& ent : ents) {
		try {
			ent.origin(this->get_filename(ent));
			account acc(ent.name().data());
			acc.origin(acc_origin);
			g.add(ent, acc);
			this->_args.erase(traits_type::name(ent));
		} catch (const std::exception& err) {
			std::cerr << gettext("error adding") << ' ';
			traits_type::print(std::cerr, ent);
			std::cerr << ": " << err.what() << std::endl;
		}
	}
}

sys::path
ggg::Add_entity::get_filename(const entity& ent) const {
	sys::path filename;
	if (this->_type == Type::Directory) {
		if (this->_path.empty()) {
			filename = "entities";
		} else {
			filename.append(this->_path);
			filename.push_back(sys::file_separator);
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

