#ifndef PAM_PAM_CALL_HH
#define PAM_PAM_CALL_HH

#include "pam_category.hh"

namespace ggg {

	namespace pam {

		inline pam_errc
		call(int ret) {
			if (ret != int(pam_errc::success)) {
				throw std::system_error(ret, pam_category);
			}
			return pam_errc(ret);
		}

	}

	inline void
	throw_pam_error(pam_errc err) {
		throw std::system_error(int(err), pam_category);
	}


}

#endif // PAM_PAM_CALL_HH
