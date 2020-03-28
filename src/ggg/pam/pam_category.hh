#ifndef GGG_PAM_PAM_CATEGORY_HH
#define GGG_PAM_PAM_CATEGORY_HH

#include <security/pam_appl.h>

#include <system_error>

#include <ggg/pam/errc.hh>

namespace pam {

    class error_category: public std::error_category {

    public:
        const char*
        name() const noexcept override {
            return "pam";
        }

        inline std::error_condition
        default_error_condition(int ev) const noexcept override;

        bool
        equivalent(const std::error_code& code, int condition)
        const noexcept override {
            return *this==code.category() && code.value() == condition;
        }

        std::string
        message(int ev) const noexcept override {
            return ::pam_strerror(nullptr, ev);
        }

    };

    extern error_category pam_category;

}

namespace std {
    inline error_condition
    make_error_condition(pam::errc e) noexcept {
        return std::error_condition(
            static_cast<int>(e),
            pam::pam_category
        );
    }
}

namespace pam {
    inline std::error_condition
    error_category::default_error_condition(int ev) const noexcept {
        return std::error_condition(ev, pam::pam_category);
    }
}
#endif // vim:filetype=cpp
