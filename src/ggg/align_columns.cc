#include "align_columns.hh"

#include <codecvt>
#include <locale>

template <class Ch>
std::vector<size_t>
ggg::align_columns(std::basic_istream<Ch>& str, Ch delimiter) {
	// calculate column widths
	std::vector<size_t> width;
	std::basic_string<Ch> line;
	while (std::getline(str, line, Ch('\n'))) {
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

template std::vector<size_t>
ggg::align_columns(std::basic_istream<char>& str, char delim);

template std::vector<size_t>
ggg::align_columns(std::basic_istream<wchar_t>& str, wchar_t delim);
