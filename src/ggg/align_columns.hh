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
	print_aligned(
		const Container& cnt,
		const std::vector<size_t>& width,
		std::ostream& out
	) {
		for (const auto& value : cnt) {
			value.print_aligned(out, width.data());
		}
	}

	std::vector<size_t>
	align_columns(std::istream& str, char delimiter) {
		// calculate column widths
		std::vector<size_t> width;
		std::string line;
		while (std::getline(str, line, '\n')) {
			size_t i0 = 0;
			size_t column_no = 0;
			const size_t line_size = line.size();
			for (size_t i=0; i<line_size; ++i) {
				if (line[i] == delimiter || i == line_size-1) {
					const size_t new_w = i-i0;
					if (column_no >= width.size()) {
						width.emplace_back(new_w);
					} else {
						const size_t w = width[column_no];
						if (new_w > w) {
							width[column_no] = new_w;
						}
					}
					++column_no;
					i0 = i;
				}
			}
		}
		return width;
	}

	template <class Container>
	void
	align_columns(const Container& cnt, std::ostream& out, char delimiter) {
		std::stringstream str;
		print_lines(cnt, str);
		std::vector<size_t> width = align_columns(str, delimiter);
		print_aligned(cnt, width, out);
	}

}

#endif // GGG_ALIGN_COLUMNS_HH
