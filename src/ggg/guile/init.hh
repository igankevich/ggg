#ifndef GGG_GUILE_INIT_HH
#define GGG_GUILE_INIT_HH

#include <libguile.h>

extern "C" void scm_init_ggg();

namespace ggg {

	void guile_init();

}

#endif // vim:filetype=cpp
