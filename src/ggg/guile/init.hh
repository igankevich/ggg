#ifndef GGG_GUILE_INIT_HH
#define GGG_GUILE_INIT_HH

#include <libguile.h>

extern "C" void scm_init_ggg();

namespace ggg {

	inline void
	guile_init() {
		scm_init_guile();
		scm_c_use_module("ggg types");
		scm_c_use_module("oop goops");
	}

}

#endif // vim:filetype=cpp
