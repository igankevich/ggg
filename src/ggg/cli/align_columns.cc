#include "align_columns.hh"

#include <ggg/bits/to_bytes.hh>

namespace ggg {

    template <>
    std::vector<size_t>
    align_columns<char>(std::basic_istream<char>& str, char delimiter) {
    	bits::wcvt_type cv;
    	std::vector<size_t> width;
    	std::string line;
    	while (std::getline(str, line, '\n')) {
    		const size_t line_size = line.size();
    		size_t i0 = 0;
    		size_t column_no = 0;
    		for (size_t i=0; i<line_size; ++i) {
    			if (line[i] == delimiter || i == line_size-1) {
    				const char* first = line.data()+i0;
    				const size_t new_w =
    					cv.from_bytes(first, line.data() + i).size();
    				if (column_no >= width.size()) {
    					width.emplace_back(new_w);
    				} else {
    					const size_t w = width[column_no];
    					if (new_w > w) {
    						width[column_no] = new_w;
    					}
    				}
    				++column_no;
    				i0 = i+1;
    			}
    		}
    	}
    	return width;
    }

}

template std::vector<size_t>
ggg::align_columns(std::basic_istream<char>& str, char delim);
