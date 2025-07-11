#pragma once

#include <fmt/core.h>

#include <string>

/**
 * @brief Return the name of this header-only library
 */
inline auto
name() -> std::string
{
    return fmt::format("{}", "gloss");
}
