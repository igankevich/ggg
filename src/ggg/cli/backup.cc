#include <ctime>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

#include <sqlitex/backup.hh>

#include <unistdx/fs/file_status>
#include <unistdx/fs/mkdirs>
#include <unistdx/fs/path>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/cli/backup.hh>
#include <ggg/cli/quiet_error.hh>

namespace {

	std::string
	destination_filename(const char* suffix, const char* format) {
		std::time_t t = std::time(nullptr);
		std::stringstream filename;
		filename.imbue(std::locale::classic());
		filename
			<< "ggg-"
			<< suffix << '-'
			<< std::put_time(std::localtime(&t), format)
			<< ".sqlite3";
		return filename.str();
	}

	struct backup_parameters {
		const char* src_path;
		const char* dst_suffix;
	};

}

void
ggg::Backup::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = ::getopt(argc, argv, "f:")) != -1) {
		if (opt == 'f') {
			this->_format = ::optarg;
		}
		if (opt == '?') {
			this->print_usage();
			throw quiet_error();
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
	if (this->_args.empty()) {
		this->print_usage();
		throw quiet_error();
	}
	if (this->_args.size() != 1) {
		throw std::runtime_error("please, specify only one directory");
	}
}

void
ggg::Backup::execute() {
	sys::path dir(*this->_args.begin());
	sys::mkdirs(dir);
	backup_parameters params[] = {
		{GGG_ENTITIES_PATH, "entities"},
		{GGG_ACCOUNTS_PATH, "accounts"}
	};
	for (const auto& param : params) {
		sys::path dst_path(dir, destination_filename(param.dst_suffix, this->_format));
		sqlite::database src(param.src_path, sqlite::open_flag::read_only);
		try {
			sys::file_status st{dst_path};
			throw std::invalid_argument("destination file already exists");
		} catch (const sys::bad_call& err) {
			if (err.errc() != std::errc::no_such_file_or_directory) {
				throw;
			}
		}
		sqlite::database dst(
			dst_path,
			sqlite::open_flag::read_write | sqlite::open_flag::create
		);
		sqlite::backup backup(src, dst);
		do {
			backup.step();
		} while (!backup.done());
	}
}

void
ggg::Backup::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-f FORMAT] DIR\n"
			  "    -f FORMAT    date format\n";
}
