#ifndef GGG_CLI_SEARCH_HH
#define GGG_CLI_SEARCH_HH

#include <regex>
#include <vector>

#include <ggg/core/database.hh>

namespace ggg {

struct Search {

    using ccvt_type = std::codecvt_utf8<wchar_t>;
    using wcvt_type = std::wstring_convert<ccvt_type, wchar_t>;

    std::vector<std::wregex> wexpr;
    wcvt_type cv;
    Database* db = nullptr;

    Search() = default;

    template <class Iterator>
    inline
    Search(Database* db, Iterator first, Iterator last) {
        this->open(db, first, last);
    }

    inline
    ~Search() {
        this->delete_function();
    }

    template <class Iterator>
    inline void
    open(Database* db, Iterator first, Iterator last) {
        this->db = db;
        while (first != last) {
            using namespace std::regex_constants;
            wexpr.emplace_back(cv.from_bytes(*first), ECMAScript | optimize | icase);
            ++first;
        }
        this->create_function();
    }

    inline int
    search(const char* field) {
        if (wexpr.empty()) { return 1; }
        std::wstring s(cv.from_bytes(field));
        for (const auto& e : wexpr) {
            if (std::regex_search(s, e)) {
                return 1;
            }
        }
        return 0;
    }

    inline void close() { this->delete_function(); }

private:

    void
    create_function();

    void
    delete_function();

};

}

#endif // vim:filetype=cpp
