#ifndef BITS_TO_BYTES_HH
#define BITS_TO_BYTES_HH

#include <codecvt>
#include <locale>

namespace ggg {

	namespace bits {

		typedef std::codecvt_utf8<wchar_t> ccvt_type;
		typedef std::wstring_convert<ccvt_type, wchar_t> wcvt_type;

	}

}

#endif // vim:filetype=cpp
