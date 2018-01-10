#include "show_base.hh"

#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/native.hh>

#include "align_columns.hh"

void
ggg::Show_base::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "qlt")) != -1) {
		switch (opt) {
			case 'q': this->_verbose = false; break;
			case 'l': this->_long = true; break;
			case 't': this->_table = true; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace(argv[i]);
	}
}

void
ggg::Show_base::execute() {
	if (this->_table) {
		for (const wentity& ent : this->_result) {
			std::wcout << ent << '\n';
		}
	} else if (this->_long) {
		align_columns(this->_result, std::wcout, wentity::delimiter, L' ', true);
		const size_t nentities = this->_result.size();
		if (nentities > 7) {
			native_message(std::wcout, "\n_ entities.\n", nentities);
		}
	} else {
		for (const wentity& ent : this->_result) {
			std::wcout << ent.name() << '\n';
		}
	}
}

void
ggg::Show_base::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-ql] ENTITY..." << '\n';
}

