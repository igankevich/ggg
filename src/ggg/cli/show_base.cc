#include <iostream>

#include <ggg/cli/align_columns.hh>
#include <ggg/cli/show_base.hh>
#include <ggg/config.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/machine.hh>
#include <ggg/core/native.hh>

template <class T>
void
ggg::Show_base<T>::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "qlt")) != -1) {
		switch (opt) {
			case 'q': this->_verbose = false; break;
			case 'l': this->_long = true; break;
			case 't': this->_table = true; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
}

template <class T>
void
ggg::Show_base<T>::execute() {
	if (this->_table) {
		for (const auto& ent : this->_result) {
			std::cout << ent << '\n';
		}
	} else if (this->_long) {
		align_columns(
			this->_result,
			std::cout,
			' ',
			true,
			entity_format::batch
		);
		const size_t nentities = this->_result.size();
		if (nentities > 7) {
			std::cout << std::endl;
			native_message_n(std::cout, nentities, "_ entities.", nentities);
		}
	} else {
		for (const auto& ent : this->_result) {
			std::cout << ent.name() << '\n';
		}
	}
}

template <class T>
void
ggg::Show_base<T>::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-ql] ENTITY..." << '\n';
}


template class ggg::Show_base<ggg::entity>;
template class ggg::Show_base<ggg::Machine>;
