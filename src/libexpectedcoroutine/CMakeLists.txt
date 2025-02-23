add_library(libexpectedcoroutine)
target_sources(libexpectedcoroutine
  PUBLIC
    FILE_SET CXX_MODULES FILES
      libexpectedcoroutine.cpp
      stdexception.cpp
      regex_error.cpp
  PRIVATE
)

target_compile_features(libexpectedcoroutine PRIVATE cxx_std_23)
set_target_properties(libexpectedcoroutine PROPERTIES CXX_EXTENSIONS OFF)
if(CXX_COMPILER_ID STREQUAL "Clang")
  set_target_properties(libexpectedcoroutine PROPERTIES CXX_CLANG_TIDY "clang-tidy")
endif()

target_compile_options(
  libexpectedcoroutine
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

target_clangformat_setup(libexpectedcoroutine)
