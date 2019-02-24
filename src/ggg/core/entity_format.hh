#ifndef GGG_CORE_ENTITY_FORMAT_HH
#define GGG_CORE_ENTITY_FORMAT_HH

#include <iosfwd>

namespace ggg {

	enum class entity_format {
		batch,
		human
	};

	typedef const size_t* width_container;

	template <class T>
	struct Entity_header {

		static char
		delimiter();

		static void
		write_header(std::ostream& out, width_container width, char delim);

		static void
		write_body(
			std::ostream& out,
			const T& ent,
			width_container width,
			entity_format fmt,
			char delim
		);

		static std::istream&
		read_body(std::istream& in, T& ent, entity_format fmt);

	};

}

#endif // vim:filetype=cpp
