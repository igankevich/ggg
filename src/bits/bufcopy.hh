#ifndef BUFCOPY_HH
#define BUFCOPY_HH

namespace ggg {

	namespace bits {

		inline char*
		bufcopy(char** field, char* dest, const char* src) {
			*field = dest;
			while ((*dest++ = *src++));
			return dest;
		}

	}

}

#endif // BUFCOPY_HH
