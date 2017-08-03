#include "edit_entity.hh"

#include <iostream>
#include <algorithm>
#include <string>
#include <set>
#include <iterator>
#include <array>
#include <vector>
#include <cstdlib>

#include <stdx/debug.hh>
#include <sys/file.hh>
#include <sys/path.hh>
#include <sys/fildesbuf.hh>
#include <sys/process.hh>

#include "config.hh"
#include "ggg.hh"
#include "align_columns.hh"
#include "bits/io.hh"

namespace {

	std::array<std::string,3> all_tmpdirs{"TMPDIR", "TMP", "TEMP"};

	std::string
	find_editor() {
		std::string result;
		if (const char* editor = std::getenv("EDITOR")) {
			result = editor;
		} else {
			result = "vi";
		}
		return result;
	}

	void
	edit_file(std::string path) {
		std::string editor = find_editor();
		sys::process child([&editor,&path] () {
			return sys::this_process::execute_command(editor, path);
		});
		sys::proc_status status = child.wait();
		if (!(status.exited() && status.exit_code() == 0)) {
			throw std::runtime_error("bad exit code from editor");
		}
	}

	sys::path
	find_temporary_directory() {
		std::string tmp;
		std::vector<std::string> possible_dirs;
		std::for_each(
			all_tmpdirs.begin(),
			all_tmpdirs.end(),
			[&possible_dirs] (const std::string& envvar) {
				if (const char* dir = std::getenv(envvar.data())) {
					possible_dirs.emplace_back(dir);
				}
			}
		);
		possible_dirs.emplace_back("/dev/shm");
		possible_dirs.emplace_back("/tmp");
		auto result = std::find_if(
			possible_dirs.begin(),
			possible_dirs.end(),
			[] (const std::string& rhs) {
				sys::file_stat st(rhs.data());
				return st.is_directory();
			}
		);
		if (result == possible_dirs.end()) {
			throw std::runtime_error("unable to find temporary directory");
		}
		return sys::path(*result);
	}

	std::string
	get_temporary_file_name_template(sys::path dir) {
		std::string basename;
		basename.append(GGG_EXECUTABLE_NAME);
		basename.push_back('.');
		basename.append("XXXXXX");
		return sys::path(dir, basename).to_string();
	}

	class Temporary_file {
		std::string _filename;
		sys::ofdstream _out;

	public:
		Temporary_file():
		_filename(get_temporary_file_name_template(find_temporary_directory())),
		_out(get_fd(_filename))
		{}

		~Temporary_file() {
			//TODO
			//this->_out.close();
			(void)std::remove(this->_filename.data());
		}

		std::ostream&
		out() noexcept {
			return this->_out;
		}

		const std::string&
		filename() const noexcept {
			return this->_filename;
		}

	private:
		static sys::fildes
		get_fd(std::string& tpl) {
			sys::fildes fd(::mkostemp(&tpl[0], O_CLOEXEC));
			if (!fd) {
				throw std::runtime_error("unable to create a file in temporary directory");
			}
			return fd;
		}

	};

}

void
ggg::Edit_entity::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "qaed")) != -1) {
		switch (opt) {
			case 'q':
				this->_verbose = false;
				break;
			case 'a':
				this->_type = Type::Account;
				break;
			case 'e':
				this->_type = Type::Entity;
				break;
			case 'd':
				this->_type = Type::Directory;
				break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace(argv[i]);
	}
}

void
ggg::Edit_entity::execute()  {
	Ggg g(GGG_ROOT, this->verbose());
	switch (this->_type) {
		case Type::Entity: {
			while (!this->_args.empty()) {
				Temporary_file tmp;
				print_entities(g, tmp.out());
				edit_file(tmp.filename());
				update_entities(g, tmp.filename());
				if (!this->_args.empty()) {
					std::clog << "press any key to continue..." << std::endl;
					std::cin.get();
				}
			}
			break;
		}
		case Type::Account: {
			Temporary_file tmp;
			print_accounts(g, tmp.out());
			break;
		}
		case Type::Directory:
			// TODO
			break;
	}
}

void
ggg::Edit_entity::print_entities(Ggg& g, std::ostream& out) {
	std::set<entity> entities;
	g.find_entities(
		this->args_begin(),
		this->args_end(),
		std::inserter(entities, entities.begin())
	);
	align_columns(entities, out, entity::delimiter);
	out.flush();
}

void
ggg::Edit_entity::print_accounts(Ggg& g, std::ostream& out) {
	std::set<account> accounts;
	g.find_accounts(
		this->args(),
		std::inserter(accounts, accounts.begin())
	);
	align_columns(accounts, out, account::delimiter);
	out.flush();
}

void
ggg::Edit_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-qaed] ENTITY...\n";
}

void
ggg::Edit_entity::update_entities(Ggg& g, const std::string& filename) {
	std::ifstream in;
	std::vector<entity> ents;
	try {
		in.exceptions(std::ios::badbit);
		in.open(filename);
		std::copy(
			std::istream_iterator<entity>(in),
			std::istream_iterator<entity>(),
			std::back_inserter(ents)
		);
	} catch (...) {
		bits::throw_io_error(in, "unable to read entities");
	}
	const size_t n = ents.size();
	for (size_t i=0; i<n; ++i) {
		const entity& lhs = ents[i];
		for (size_t j=i+1; j<n; ++j) {
			const entity& rhs = ents[j];
			if (lhs.id() == rhs.id() || lhs.name() == rhs.name()) {
				throw std::invalid_argument("duplicate names/ids");
			}
		}
	}
	for (const entity& ent : ents) {
		try {
			g.update(ent);
			this->_args.erase(ent.name());
		} catch (const std::exception& err) {
			std::cerr << "error updating "
				<< ent.name() << ':' << ent.id()
				<< ": " << err.what() << std::endl;
		}
	}
}
