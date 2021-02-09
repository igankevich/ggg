#ifndef GGG_CORE_TIES_HH
#define GGG_CORE_TIES_HH

namespace ggg {

    enum class Ties {
        User_user=0,
        User_group=1,
        Group_group=2,
    };

    Ties string_to_ties(const char* s);
    const char* ties_to_string(Ties t) noexcept;

}

#endif // vim:filetype=cpp
