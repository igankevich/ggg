#include <ggg/core/account.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/group.hh>
#include <ggg/core/machine.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/password.hh>

extern "C" void
scm_init_ggg() {
	using namespace ggg;
	Guile_traits<Machine>::define_procedures();
	Guile_traits<entity>::define_procedures();
	Guile_traits<::ggg::group>::define_procedures();
	Guile_traits<account>::define_procedures();
	password_define_procedures();
}
