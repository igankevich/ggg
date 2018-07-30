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
		for (const entity& ent : this->_result) {
			std::cout << ent << '\n';
		}
	} else if (this->_long) {
		align_columns(this->_result, std::cout, entity::delimiter, ' ', true);
		const size_t nentities = this->_result.size();
		if (nentities > 7) {
			std::cout << std::endl;
			native_message_n(std::cout, nentities, "_ entities.", nentities);
		}
	} else {
		for (const entity& ent : this->_result) {
			std::cout << ent.name() << '\n';
		}
	}
}

void
ggg::Show_base::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-ql] ENTITY..." << '\n';
}

