#include <ggg/pam/handle.hh>

auto
pam::handle::error(const std::system_error& e, errc def) const -> errc {
    errc ret;
    if (e.code().category() == std::iostream_category()) {
        ret = def;
        pam_syslog(*this, LOG_CRIT, "%s", e.what());
    } else {
        if (e.code().category() == pam::pam_category) {
            ret = errc(e.code().value());
        } else {
            ret = def;
        }
        int level = (e.code().value() == PAM_PERM_DENIED) ? LOG_NOTICE : LOG_ERR;
        pam_syslog(*this, level, "%s", e.what());
    }
    return ret;
}
