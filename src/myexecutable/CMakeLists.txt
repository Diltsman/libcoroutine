add_library(myexecutable)
target_sources(myexecutable
  PRIVATE
      main.cpp
)

target_compile_features(myexecutable PRIVATE cxx_std_23)
set_target_properties(myexecutable PROPERTIES CXX_EXTENSIONS OFF
                                              CXX_CLANG_TIDY "clang-tidy")

target_link_libraries(myexecutable PRIVATE libmy)

target_compile_options(
  myexecutable
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
          $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

target_clangformat_setup(myexecutable)
