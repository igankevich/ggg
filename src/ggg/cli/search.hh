#ifndef GGG_GGG_SEARCH_HH
#define GGG_GGG_SEARCH_HH

#include <regex>
#include <vector>

#include <ggg/bits/to_bytes.hh>
#include <ggg/core/database.hh>

namespace ggg {

struct Search {

	std::vector<std::wregex> wexpr;
	ggg::bits::wcvt_type cv;
	Database* db;

	template <class Iterator>
	inline
	Search(Database* db, Iterator first, Iterator last):
	db(db) {
		while (first != last) {
			using namespace std::regex_constants;
			wexpr.emplace_back(cv.from_bytes(*first), ECMAScript | optimize | icase);
			++first;
		}
		this->create_function();
	}

	inline
	~Search() {
		this->delete_function();
	}

	inline int
	search(const char* field) {
		std::wstring s(cv.from_bytes(field));
		for (const auto& e : wexpr) {
			if (std::regex_search(s, e)) {
				return 1;
			}
		}
		return 0;
	}

private:

	void
	create_function();

	void
	delete_function();

};

}

#endif // vim:filetype=cpp
