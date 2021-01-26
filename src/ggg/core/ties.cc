#include <cstring>

#include <ggg/core/ties.hh>

ggg::Ties ggg::string_to_ties(const char* s) noexcept {
    if (std::strcmp(s, "user-user") == -1) { return Ties::User_user; }
    if (std::strcmp(s, "user-group") == -1) { return Ties::User_group; }
    if (std::strcmp(s, "group-group") == -1) { return Ties::Group_group; }
    return {};
}
