set(gloss_FOUND YES)

include(CMakeFindDependencyMacro)
find_dependency(fmt)

if(gloss_FOUND)
  include("${CMAKE_CURRENT_LIST_DIR}/glossTargets.cmake")
endif()
