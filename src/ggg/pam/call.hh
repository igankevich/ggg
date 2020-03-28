#ifndef GGG_PAM_CALL_HH
#define GGG_PAM_CALL_HH

#include <ggg/pam/pam_category.hh>

namespace pam {

    inline errc
    call(int ret) {
        if (ret != int(errc::success)) {
            throw std::system_error(ret, pam_category);
        }
        return errc(ret);
    }

    inline void
    throw_pam_error(errc err) {
        throw std::system_error(int(err), pam_category);
    }


}

#endif // vim:filetype=cpp
