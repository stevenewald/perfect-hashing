#pragma once

#include <cstdint>

namespace rndom {
// https://www.pcg-random.org
struct pcg {
    [[nodiscard]] constexpr auto
    operator()() noexcept -> std::uint32_t
    {
        std::uint64_t old_state = state;
        state = old_state * 6364136223846793005u + increment;
        std::uint32_t xor_shifted = ((old_state >> 18u) ^ old_state) >> 27u;
        auto rot = static_cast<std::uint32_t>(old_state >> 59u);
        return (xor_shifted >> rot) | (xor_shifted << ((-rot) & 31u));
    }

    std::uint64_t increment{1442695040888963407u};
    std::uint64_t state{5573589319906701683u + increment};
};
} // namespace rndom
