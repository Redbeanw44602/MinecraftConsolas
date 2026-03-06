#pragma once

#include <cstdint>
#include <cstdlib>

struct LegacyRNG {
    using result_type = std::int32_t;

    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return RAND_MAX; }

    result_type operator()() const { return std::rand(); }
};
