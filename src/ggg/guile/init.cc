#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/group.hh>
#include <ggg/core/machine.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/init.hh>
#include <ggg/guile/password.hh>
#include <ggg/guile/store.hh>

extern "C" void
scm_init_ggg() {
	using namespace ggg;
	Guile_traits<Machine>::define_procedures();
	Guile_traits<entity>::define_procedures();
	Guile_traits<::ggg::group>::define_procedures();
	Guile_traits<account>::define_procedures();
	password_define_procedures();
	store_define_procedures();
}

void
ggg::guile_init() {
    scm_init_guile();
    scm_c_eval_string("(add-to-load-path \"" GGG_GUILE_LOAD_PATH "\")");
    scm_c_use_module("ggg types");
    scm_c_use_module("oop goops");
}

