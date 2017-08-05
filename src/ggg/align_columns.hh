#ifndef GGG_ALIGN_COLUMNS_HH
#define GGG_ALIGN_COLUMNS_HH

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace ggg {

	template <class Container>
	void
	print_lines(const Container& cnt, std::ostream& out) {
		for (const auto& value : cnt) {
			out << value << '\n';
		}
	}

	template <class Container>
	void
	write_human(
		const Container& cnt,
		const std::vector<size_t>& width,
		std::ostream& out
	) {
		for (const auto& value : cnt) {
			value.write_human(out, width.data());
		}
	}

	std::vector<size_t>
	align_columns(std::istream& str, char delimiter);

	template <class Container>
	void
	align_columns(const Container& cnt, std::ostream& out, char delimiter) {
		std::stringstream str;
		print_lines(cnt, str);
		std::vector<size_t> width = align_columns(str, delimiter);
		write_human(cnt, width, out);
	}

}

#endif // GGG_ALIGN_COLUMNS_HH
