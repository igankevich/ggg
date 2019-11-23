#include <unistd.h>

#include <iostream>

#include <ggg/cli/show_base.hh>
#include <ggg/cli/cli_traits.hh>
#include <ggg/config.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/machine.hh>
#include <ggg/core/native.hh>

template <class T>
void
ggg::Show_base<T>::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "qt:o:")) != -1) {
		switch (opt) {
			case 'q': this->_verbose = false; break;
			case 't': std::string(::optarg) >> this->_type; break;
			case 'o': std::string(::optarg) >> this->_oformat; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
}

template <class T>
void
ggg::Show_base<T>::execute() {
    write<T>(std::cout, this->_result, this->_oformat);
    this->_result.close();
}

template class ggg::Show_base<ggg::entity>;
template class ggg::Show_base<ggg::Machine>;
