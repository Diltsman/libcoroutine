add_executable(libexpectedcoroutinetest)
target_sources(libexpectedcoroutinetest
  PRIVATE
      libexpectedcoroutinetest.cpp
)

target_compile_features(libexpectedcoroutinetest PRIVATE cxx_std_23)
set_target_properties(libexpectedcoroutinetest PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(libexpectedcoroutinetest PRIVATE libexpectedcoroutine Catch2::Catch2WithMain)

target_clangformat_setup(libexpectedcoroutinetest)

catch_discover_tests(libexpectedcoroutinetest)
