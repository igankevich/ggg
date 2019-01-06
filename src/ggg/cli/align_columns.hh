#ifndef GGG_ALIGN_COLUMNS_HH
#define GGG_ALIGN_COLUMNS_HH

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

#include <ggg/core/entity_format.hh>

namespace ggg {

	template <class Container, class Ch>
	void
	print_lines(const Container& cnt, std::basic_ostream<Ch>& out) {
		for (const auto& value : cnt) {
			out << value << '\n';
		}
	}

	template <class Container, class Ch>
	void
	write_human(
		const Container& cnt,
		const std::vector<size_t>& width,
		std::basic_ostream<Ch>& out,
		Ch delimiter,
		entity_format format
	) {
		for (const auto& value : cnt) {
			value.write_human(out, width.data(), format, delimiter);
		}
	}

	template <class Ch>
	std::vector<size_t>
	align_columns(std::basic_istream<Ch>& str, Ch delimiter);

	template <class Container, class Ch>
	void
	align_columns(
		const Container& cnt,
		std::basic_ostream<Ch>& out,
		Ch delimiter,
		Ch out_delimiter,
		bool header,
		entity_format format
	) {
		typedef typename Container::value_type entity_type;
		std::basic_stringstream<Ch> str;
		str.imbue(std::locale::classic());
		if (header) {
			entity_type::write_header(str, nullptr, 0);
		}
		print_lines(cnt, str);
		std::vector<size_t> width = align_columns(str, delimiter);
		if (header) {
			entity_type::write_header(out, width.data(), out_delimiter);
		}
		write_human(cnt, width, out, out_delimiter, format);
	}

	template <class Container, class Ch>
	void
	align_columns(
		const Container& cnt,
		std::basic_ostream<Ch>& out,
		Ch delimiter,
		entity_format format
	) {
		align_columns(cnt, out, delimiter, delimiter, false, format);
	}
}

#endif // GGG_ALIGN_COLUMNS_HH
