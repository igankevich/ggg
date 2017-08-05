#include "align_columns.hh"

std::vector<size_t>
ggg::align_columns(std::istream& str, char delimiter) {
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

