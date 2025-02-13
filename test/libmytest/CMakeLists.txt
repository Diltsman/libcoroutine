add_executable(libmytest)
target_sources(libmytest
  PRIVATE
      libmytest.cpp
)

target_compile_features(libmytest PRIVATE cxx_std_23)
set_target_properties(libmytest PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(libmytest PRIVATE libmy Catch2::Catch2WithMain)

target_clangformat_setup(libmytest)

catch_discover_tests(libmytest)
