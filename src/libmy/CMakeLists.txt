
add_library(libmy)
target_sources(libmy
  PUBLIC
    FILE_SET CXX_MODULES FILES
      libmy.cpp
)

target_compile_features(libmy PRIVATE cxx_std_23)
set_target_properties(libmy PROPERTIES CXX_EXTENSIONS OFF)
if(CXX_COMPILER_ID STREQUAL "Clang")
  set_target_properties(libmy PROPERTIES CXX_CLANG_TIDY "clang-tidy")
endif()

target_compile_options(
  libmy
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

target_clangformat_setup(libmy)
