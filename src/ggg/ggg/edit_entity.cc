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

#include <ggg/config.hh>
#include <ggg/ctl/ggg.hh>
#include "editor.hh"
#include "tmpfile.hh"
#include "align_columns.hh"
#include "object_traits.hh"
#include <ggg/core/lock.hh>
#include <ggg/core/native.hh>
#include <ggg/ggg/quiet_error.hh>

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
	file_lock lock;
	GGG g(GGG_ENT_ROOT, this->verbose());
	lock.unlock();
	switch (this->_type) {
		case Type::Entity: this->edit_objects<entity>(g); break;
		case Type::Account: this->edit_objects<account>(g); break;
		case Type::Directory: this->edit_directory(); break;
	}
}

template <class T>
void
ggg::Edit_entity::edit_objects(GGG& g) {
	if (this->is_batch()) {
		this->edit_batch<T>(g);
	} else {
		this->edit_interactive<T>(g);
	}
}

template <class T>
void
ggg::Edit_entity::edit_interactive(GGG& g) {
	while (!this->_args.empty()) {
		sys::tmpfile tmp;
		tmp.out().imbue(std::locale::classic());
		this->print_objects<T>(g, tmp.out());
		edit_file_or_throw(tmp.filename());
		this->update_objects<T>(g, tmp.filename());
		if (!this->_args.empty()) {
			native_message(std::wclog, "Press any key to continue...");
			std::wcin.get();
		}
	}
}

template <class T>
void
ggg::Edit_entity::edit_batch(GGG& g) {
	sys::tmpfile tmp;
	tmp.out().imbue(std::locale::classic());
	tmp.out() << std::cin.rdbuf();
	tmp.out().flush();
	this->update_objects<T>(g, tmp.filename());
}

template <class T>
void
ggg::Edit_entity::print_objects(GGG& g, std::ostream& out) {
	file_lock lock;
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
	file_lock lock(true);
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
			g.update(ent);
			this->_args.erase(traits_type::name(ent));
		} catch (const std::exception& err) {
			++nerrors;
			bits::wcvt_type cv;
			std::stringstream tmp;
			tmp.imbue(std::locale::classic());
			tmp << ent;
			native_message(
				std::wcerr,
				"error updating _: _",
				cv.from_bytes(tmp.str()),
				wnative(err.what(), cv)
			);
		}
	}
	if (nerrors > 0) {
		throw quiet_error();
	}
}

void
ggg::Edit_entity::edit_directory() {
	sys::proc_status status;
	bool success;
	do {
		status = ::ggg::edit_directory(sys::path(GGG_ENT_ROOT));
		success = status.exited() && status.exit_code() == 0;
		if (!success) {
			native_message(std::wcerr, "bad exit code from editor");
			native_message(std::wcerr, "Press any key to continue...");
			std::wcin.get();
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
template void
ggg::Edit_entity::edit_batch<ggg::entity>(GGG& g);
template void
ggg::Edit_entity::edit_batch<ggg::account>(GGG& g);
template void
ggg::Edit_entity::edit_interactive<ggg::entity>(GGG& g);
template void
ggg::Edit_entity::edit_interactive<ggg::account>(GGG& g);
