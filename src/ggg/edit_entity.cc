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

	template <class T>
	struct Object_traits;

	template <>
	struct Object_traits<ggg::entity> {

		static const std::string&
		name(const ggg::entity& rhs) noexcept {
			return rhs.name();
		}

		static bool
		eq(const ggg::entity& lhs, const ggg::entity& rhs) noexcept {
			return lhs.id() == rhs.id() || lhs.name() == rhs.name();
		}

		static std::ostream&
		print(std::ostream& out, const ggg::entity& rhs) {
			return out << rhs.name() << ':' << rhs.id();
		}

		static char
		delimiter() noexcept {
			return ggg::entity::delimiter;
		}

		template <class Container, class Result>
		static void
		find(ggg::Ggg& g, const Container& args, Result result) {
			g.find_entities(args.begin(), args.end(), result);
		}

	};

	template <>
	struct Object_traits<ggg::account> {

		static const char*
		name(const ggg::account& rhs) noexcept {
			return rhs.login().data();
		}

		static bool
		eq(const ggg::account& lhs, const ggg::account& rhs) noexcept {
			return lhs.login() == rhs.login();
		}

		static std::ostream&
		print(std::ostream& out, const ggg::account& rhs) {
			return out << rhs.login();
		}

		static char
		delimiter() noexcept {
			return ggg::account::delimiter;
		}

		template <class Container, class Result>
		static void
		find(ggg::Ggg& g, const Container& args, Result result) {
			g.find_accounts(args, result);
		}

	};

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

	template <class T, class Result>
	void
	read_objects(const std::string& filename, Result result, const char* msg) {
		std::ifstream in;
		try {
			in.exceptions(std::ios::badbit);
			in.open(filename);
			T obj;
			while (obj.read_formatted(in)) {
				*result++ = obj;
			}
		} catch (...) {
			ggg::bits::throw_io_error(in, msg);
		}
	}

	template <class Container, class BinOp>
	void
	check_duplicates(const Container& objs, BinOp op) {
		const size_t n = objs.size();
		for (size_t i=0; i<n; ++i) {
			const auto& lhs = objs[i];
			for (size_t j=i+1; j<n; ++j) {
				const auto& rhs = objs[j];
				if (op(lhs, rhs)) {
					throw std::invalid_argument("duplicate names/ids");
				}
			}
		}
	}

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
				print_objects<entity>(g, tmp.out());
				edit_file(tmp.filename());
				update_objects<ggg::entity>(g, tmp.filename());
				if (!this->_args.empty()) {
					std::clog << "press any key to continue..." << std::endl;
					std::cin.get();
				}
			}
			break;
		}
		case Type::Account: {
			while (!this->_args.empty()) {
				Temporary_file tmp;
				print_objects<account>(g, tmp.out());
				edit_file(tmp.filename());
				update_objects<ggg::account>(g, tmp.filename());
				if (!this->_args.empty()) {
					std::clog << "press any key to continue..." << std::endl;
					std::cin.get();
				}
			}
			break;
		}
		case Type::Directory:
			// TODO
			break;
	}
}

template <class T>
void
ggg::Edit_entity::print_objects(Ggg& g, std::ostream& out) {
	typedef Object_traits<T> traits_type;
	std::set<T> cnt;
	traits_type::find(g, this->args(), std::inserter(cnt, cnt.begin()));
	align_columns(cnt, out, traits_type::delimiter());
	out.flush();
}

void
ggg::Edit_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-qaed] ENTITY...\n";
}

template <class T>
void
ggg::Edit_entity::update_objects(Ggg& g, const std::string& filename) {
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

template void
ggg::Edit_entity::print_objects<ggg::entity>(Ggg& g, std::ostream& out);
template void
ggg::Edit_entity::print_objects<ggg::account>(Ggg& g, std::ostream& out);
template void
ggg::Edit_entity::update_objects<ggg::entity>(Ggg& g, const std::string& filename);
template void
ggg::Edit_entity::update_objects<ggg::account>(Ggg& g, const std::string& filename);
