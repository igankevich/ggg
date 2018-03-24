#ifndef BITS_TO_BYTES_HH
#define BITS_TO_BYTES_HH

#include <codecvt>

namespace ggg {

	namespace bits {

		typedef std::codecvt_utf8<wchar_t> ccvt_type;
		typedef std::wstring_convert<ccvt_type, wchar_t> wcvt_type;

		template <class Ch>
		std::basic_string<Ch>
		to_bytes(wcvt_type& cv, const std::basic_string<wchar_t>& rhs);

		template <>
		inline std::basic_string<char>
		to_bytes<char>(wcvt_type& cv, const std::basic_string<wchar_t>& rhs) {
			return cv.to_bytes(rhs);
		}

		template <>
		inline std::basic_string<wchar_t>
		to_bytes<wchar_t>(wcvt_type&, const std::basic_string<wchar_t>& rhs) {
			return rhs;
		}

		template <class Ch>
		std::basic_string<Ch>
		to_bytes(wcvt_type& cv, const std::basic_string<char>& rhs);

		template <>
		inline std::basic_string<char>
		to_bytes<char>(wcvt_type& cv, const std::basic_string<char>& rhs) {
			return rhs;
		}

		template <>
		inline std::basic_string<wchar_t>
		to_bytes<wchar_t>(wcvt_type& cv, const std::basic_string<char>& rhs) {
			return cv.from_bytes(rhs);
		}

	}

}

#endif // vim:filetype=cpp
