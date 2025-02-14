cmake_minimum_required(VERSION 3.29 FATAL_ERROR)

project(libcoroutine)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake
                              ${PROJECT_BINARY_DIR}/_deps/catch2-src/extras
                              ${PROJECT_BINARY_DIR}/_deps/clangformat-cmake-src/cmake)

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(
    FATAL_ERROR
      "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles."
  )
endif()

# Update all git submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE GIT_SUBMOD_RESULT)
  if(NOT GIT_SUBMOD_RESULT EQUAL "0")
    message(
      FATAL_ERROR
        "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
    )
  endif()
endif()

include(CPM)
CPMAddPackage(
  NAME Catch2
  GITHUB_REPOSITORY catchorg/Catch2
  GIT_TAG v3.8.0)
CPMAddPackage(
  NAME clangformat-cmake
  GITHUB_REPOSITORY Diltsman/clangformat-cmake
  GIT_TAG 4fef9929bb3b239c94ee6557de03bb1b5899e65a)

include(CTest)
include(Catch)

include(ClangFormat)

add_subdirectory(src)
add_subdirectory(test)
