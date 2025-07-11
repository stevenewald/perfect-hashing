cmake_minimum_required(VERSION 3.14)

project(glossTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(gloss REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(gloss_test source/gloss_test.cpp)
target_link_libraries(
    gloss_test PRIVATE
    gloss::gloss
    Catch2::Catch2WithMain
)
target_compile_features(gloss_test PRIVATE cxx_std_23)

catch_discover_tests(gloss_test)

# ---- End-of-file commands ----

add_folders(Test)
