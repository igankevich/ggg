#ifndef GGG_SEC_RANDOM_HH
#define GGG_SEC_RANDOM_HH

#include <cstdint>

#include <sodium.h>

namespace ggg {

    class unpredictable_random_engine {

    public:
        using result_type = std::uint32_t;

    public:
        inline result_type operator()() { return ::randombytes_random(); }
        inline void stir() { ::randombytes_stir(); }

        static constexpr result_type min() {
            return std::numeric_limits<result_type>::min();
        }

        static constexpr result_type max() {
            return std::numeric_limits<result_type>::max();
        }

    };

}

#endif // vim:filetype=cpp
