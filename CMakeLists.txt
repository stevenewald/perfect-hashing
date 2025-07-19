cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    gloss
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(gloss_gloss INTERFACE)
add_library(gloss::gloss ALIAS gloss_gloss)

set_property(
    TARGET gloss_gloss PROPERTY
    EXPORT_NAME gloss
)

target_include_directories(
    gloss_gloss ${warning_guard}
    INTERFACE
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
)

target_compile_features(gloss_gloss INTERFACE cxx_std_23)

find_package(fmt REQUIRED)
target_link_libraries(gloss_gloss INTERFACE fmt::fmt)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT gloss_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of gloss"
  )
endif()

include(cmake/dev-mode.cmake)
