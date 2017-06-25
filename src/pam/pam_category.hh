#ifndef PAM_PAM_ERROR_CATEGORY_HH
#define PAM_PAM_ERROR_CATEGORY_HH

#include <system_error>
#include <security/pam_appl.h>
#include "pam_errc.hh"

namespace ggg {

	class pam_error_catergory: public std::error_category {

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

	extern pam_error_catergory pam_category;

}

namespace std {
	inline error_condition
	make_error_condition(ggg::pam_errc e) noexcept {
		return std::error_condition(
			static_cast<int>(e),
			ggg::pam_category
		);
	}
}

namespace ggg {
	inline std::error_condition
	pam_error_catergory::default_error_condition(int ev) const noexcept {
		return std::error_condition(ev, ggg::pam_category);
	}
}
#endif // PAM_PAM_ERROR_CATEGORY_HH
