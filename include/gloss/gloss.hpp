#pragma once

#include <fmt/core.h>

#include <array>
#include <functional>
#include <source_location>
#include <string>
#include <utility>

inline auto
name() -> std::string
{
    return fmt::format("{}", "gloss");
}

template <typename P>
concept Pair = requires {
    typename P::first_type;
    typename P::second_type;
};

template <typename R>
concept PairRange =
    std::ranges::forward_range<R> && Pair<std::ranges::range_value_t<R>>;

// for shorthands
template <const auto& Table>
requires PairRange<decltype(Table)>
struct lookup_lut {
    using first_type = std::ranges::range_value_t<decltype(Table)>::first_type;
    using second_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    constexpr second_type
    lookup(const first_type& search_key)
    {
        for (const auto& [key, value] : Table) {
            if (key == search_key)
                return value;
        };
        std::unreachable();
    }
};

template <const auto& Table>
auto
lookup(const auto& search_key)
{
    lookup_lut<Table> table{};
    return table.lookup(search_key);
}
