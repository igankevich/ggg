#include <unistd.h>

#include <ggg/cli/select_base.hh>

void
ggg::Select_base::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "t:o:")) != -1) {
		switch (opt) {
			case 't': std::string(::optarg) >> this->_type; break;
			case 'o': std::string(::optarg) >> this->_oformat; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
}

