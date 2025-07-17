#pragma once

#include <cassert>
#include <cstdint>

#include <algorithm>
#include <array>
#include <bit>
#include <ranges>
#include <string>

namespace gloss {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

namespace random {
// https://www.pcg-random.org
struct pcg {
    [[nodiscard]] constexpr auto
    operator()() noexcept -> std::uint32_t
    {
        std::uint64_t old_state = state_;
        state_ = old_state * 6364136223846793005u + increment_;
        auto xor_shifted =
            static_cast<std::uint32_t>(((old_state >> 18u) ^ old_state) >> 27u);
        auto rot = static_cast<std::uint32_t>(old_state >> 59u);
        return (xor_shifted >> rot) | (xor_shifted << ((32u - rot) & 31u));
    }

private:
    std::uint64_t increment_{1442695040888963407u};
    std::uint64_t state_{5573589319906701683u + increment_};
};
} // namespace random

template <typename T>
constexpr T
to(const auto& data)
{
    // using data_type = decltype(data);
    if constexpr (requires { data.size(); }) {
        T tmp{};
        for (std::size_t i = 0; i < data.size(); ++i) {
            tmp |= static_cast<T>(static_cast<T>(data[i]) << (i * __CHAR_BIT__));
        }
        return tmp;
    }
    else if constexpr (requires(u32 n) { data[n]; }) {
        T tmp{};
        for (std::size_t i = 0; i < sizeof(T) && data[i] != '\0'; ++i) {
            tmp |= static_cast<T>(static_cast<T>(data[i]) << (i * __CHAR_BIT__));
        }
        return tmp;
    }
    else if constexpr (std::same_as<T, std::string>) {
        std::size_t str_size{};
        std::string chars(sizeof(decltype(data)), '\0');
        for (std::size_t i = 0; i < chars.size(); ++i) {
            chars[i] = static_cast<char>(data >> (i * __CHAR_BIT__));
            if ((data >> (i * __CHAR_BIT__)) != 0) {
                str_size = i + 1;
            }
        }
        chars.resize(str_size);
        return chars;
    }
    else {
        return static_cast<T>(data);
    }
}

template <const auto& Table>
struct entries {
    using pair_type = std::ranges::range_value_t<decltype(Table)>;
    static constexpr auto SIZE = Table.size();

    // Support string_view, const char*, and integral keys
    using key_type = decltype([]() {
        if constexpr (requires { Table[0].first.size(); }) {
            static constexpr auto MAX_SIZE = []() {
                std::size_t max{};
                for (std::size_t i = 0; i < SIZE; ++i) {
                    max = std::max(max, Table[i].first.size());
                }
                return max;
            }();

            if constexpr (MAX_SIZE <= sizeof(u8))
                return u8{};
            else if constexpr (MAX_SIZE <= sizeof(u16))
                return u16{};
            else if constexpr (MAX_SIZE <= sizeof(u32))
                return u32{};
            else if constexpr (MAX_SIZE <= sizeof(u64))
                return u64{};
        }
        else if constexpr (std::is_same_v<
                               typename pair_type::first_type, const char*>) {
            static constexpr auto MAX_SIZE = []() {
                std::size_t max{};
                for (std::size_t i = 0; i < SIZE; ++i) {
                    const char* tmp = Table[i].first;
                    std::size_t len{};
                    while (*tmp != '\0') {
                        ++tmp;
                        ++len;
                    }
                    max = std::max(max, len);
                }
                return max;
            }();

            if constexpr (MAX_SIZE <= sizeof(u8))
                return u8{};
            else if constexpr (MAX_SIZE <= sizeof(u16))
                return u16{};
            else if constexpr (MAX_SIZE <= sizeof(u32))
                return u32{};
            else if constexpr (MAX_SIZE <= sizeof(u64))
                return u64{};
        }
        else {
            return typename pair_type::first_type{};
        }
    }());

    // Support string and integral values
    using mapped_type = decltype([]() {
        if constexpr (std::is_same_v<typename pair_type::second_type, std::string>) {
            static constexpr auto MAX_SIZE = []() {
                std::size_t max{};
                for (std::size_t i = 0; i < SIZE; ++i) {
                    max = std::max(max, Table[i].second.size());
                }
                return max;
            }();

            if constexpr (MAX_SIZE <= sizeof(u8))
                return u8{};
            else if constexpr (MAX_SIZE <= sizeof(u16))
                return u16{};
            else if constexpr (MAX_SIZE <= sizeof(u32))
                return u32{};
            else if constexpr (MAX_SIZE <= sizeof(u64))
                return u64{};
        }
        else {
            return typename pair_type::second_type{};
        }
    }());

    static constexpr auto MAPPINGS = []() {
        std::array<std::pair<key_type, mapped_type>, SIZE> entries;
        for (std::size_t i = 0; i < SIZE; ++i) {
            entries[i] = std::pair<key_type, mapped_type>{
                to<key_type>(Table[i].first), to<mapped_type>(Table[i].second)
            };
        }
        return entries;
    }();
};

template <typename P>
concept Pair = requires {
    typename P::first_type;
    typename P::second_type;
};

template <typename R>
concept PairRange =
    std::ranges::forward_range<R> && Pair<std::ranges::range_value_t<R>>;

template <const auto& Table, typename ValueType>
requires PairRange<decltype(Table)>
struct lookup_lut {
    using key_type = entries<Table>::key_type;
    using mapped_type = entries<Table>::mapped_type;
    using result_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    consteval explicit lookup_lut(std::uint32_t max_attempts = 100'000) noexcept
    {
        random::pcg rand_pcg{};

        // Values won't fit in the LUT
        if (NBITS * Table.size() > (sizeof(ValueType) * __CHAR_BIT__))
            return;

        auto attempt_find_perfect_hash = [&]() {
            magic_ = rand_pcg();
            for (const auto& [key, value] : entries<Table>::MAPPINGS) {
                u32 shift = u32((key * magic_) >> SHIFT);
                if (shift >= sizeof(ValueType) * __CHAR_BIT__) {
                    lut_ = {};
                    return;
                }

                lut_ |= ValueType(value) << shift;
            }

            for (const auto& [key, value] : entries<Table>::MAPPINGS) {
                if ((lut_ >> (ValueType(key) * magic_ >> SHIFT) & MASK) != value) {
                    lut_ = {};
                    return;
                }
            }
        };

        while (lut_ == 0 && max_attempts-- > 0) {
            attempt_find_perfect_hash();
        }
    }

    constexpr explicit
    operator bool() const noexcept
    {
        return MASK and SHIFT and magic_ and lut_;
    }

    constexpr result_type
    operator()(const auto& search_key) const noexcept
    {
        return to<result_type>(
            (lut_ >> ((to<key_type>(search_key) * magic_) >> SHIFT)) & MASK
        );
    }

private:
    static constexpr ValueType MAX_BITS = []() {
        u32 max = 0;
        for (const auto& pair : entries<Table>::MAPPINGS) {
            max = std::max(static_cast<u32>(pair.second), max);
        }
        // std::countl_zero counts differently, so use builtin
        return std::countl_zero(max);
    }();
    static constexpr ValueType NBITS = (sizeof(u32) * __CHAR_BIT__) - MAX_BITS;
    static constexpr ValueType MASK = (1uz << NBITS) - 1uz;
    static constexpr ValueType SHIFT = (sizeof(u32) * __CHAR_BIT__) - NBITS;

    ValueType magic_{};
    ValueType lut_{};
};

template <const auto& Table>
requires PairRange<decltype(Table)>
struct lookup_array {
    using ValueType = u64;

    using key_type = entries<Table>::key_type;
    using mapped_type = entries<Table>::mapped_type;
    using result_type = std::ranges::range_value_t<decltype(Table)>::second_type;

    consteval explicit lookup_array(std::uint32_t max_attempts = 100'000) noexcept
    {
        random::pcg rand_pcg{};

        auto attempt_find_perfect_hash = [&]() {
            magic_ = rand_pcg();
            for (const auto& [key, value] : entries<Table>::MAPPINGS) {
                u32 shift = u32(((key * magic_) >> SHIFT) & SIZE_MASK);
                if (shift >= SIZE) {
                    table_ = {};
                    magic_ = {};
                    return;
                }

                table_[shift] = to<mapped_type>(value);
            }

            for (const auto& [key, value] : entries<Table>::MAPPINGS) {
                if (table_[(ValueType(key) * magic_ >> SHIFT) & SIZE_MASK]
                    != to<mapped_type>(value)) {
                    table_ = {};
                    magic_ = {};
                    return;
                }
            }
        };

        while (magic_ == 0 && max_attempts-- > 0) {
            attempt_find_perfect_hash();
        }
    }

    constexpr explicit
    operator bool() const noexcept
    {
        return magic_ != 0;
    }

    constexpr result_type
    operator()(const auto& search_key) const noexcept
    {
        return to<result_type>(
            table_[((to<key_type>(search_key) * magic_ >> SHIFT)) & SIZE_MASK]
        );
    }

private:
    static constexpr std::size_t SIZE = Table.size();
    static constexpr ValueType MAX_BITS = []() {
        u32 max = 0;
        for (const auto& pair : entries<Table>::MAPPINGS) {
            max = std::max(static_cast<u32>(pair.second), max);
        }
        // std::countl_zero counts differently, so use builtin
        return std::countl_zero(max);
    }();
    static constexpr ValueType NBITS = (sizeof(u32) * __CHAR_BIT__) - MAX_BITS;
    static constexpr ValueType SHIFT = (sizeof(u32) * __CHAR_BIT__) - NBITS;
    static constexpr u64 SIZE_MASK =
        (1u << static_cast<unsigned>(std::bit_width(SIZE - 1))) - 1u;

    ValueType magic_{};
    std::array<mapped_type, SIZE> table_{};
};

enum class LookupMethod : std::uint8_t { word, array, any };

template <const auto& Table, LookupMethod Method = LookupMethod::any>
constexpr auto
lookup(const auto& search_key)
{
    if constexpr (Method != LookupMethod::array) {
        if constexpr (constexpr lookup_lut<Table, u32> TABLE32{}; TABLE32) {
            return TABLE32(search_key);
        }
        else if constexpr (constexpr lookup_lut<Table, u64> TABLE64{}; TABLE64) {
            return TABLE64(search_key);
        }
    }
    else if constexpr (Method != LookupMethod::word) {
        if constexpr (constexpr lookup_array<Table> TABLE_ARRAY{}; TABLE_ARRAY) {
            return TABLE_ARRAY(search_key);
        }
    }
}

} // namespace gloss
