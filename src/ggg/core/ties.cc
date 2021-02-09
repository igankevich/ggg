#include <cstring>
#include <stdexcept>

#include <ggg/core/ties.hh>

ggg::Ties ggg::string_to_ties(const char* s) {
    if (std::strcmp(s, "user-user") == 0) { return Ties::User_user; }
    if (std::strcmp(s, "user-group") == 0) { return Ties::User_group; }
    if (std::strcmp(s, "group-group") == 0) { return Ties::Group_group; }
    throw std::invalid_argument("bad ties type");
}

const char* ggg::ties_to_string(Ties t) noexcept {
    switch (t) {
        case Ties::User_user: return "user-user";
        case Ties::User_group: return "user-group";
        case Ties::Group_group: return "group-group";
        default: return nullptr;
    }
}
