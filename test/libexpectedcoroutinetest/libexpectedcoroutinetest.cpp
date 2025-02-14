import libexpectedcoroutine;

#include <catch2/catch_all.hpp>
#include <coroutine>

TEST_CASE("co_return returns expected value", "[expectedcoroutine]") {
  REQUIRE([]() -> exco::result_t<int> { co_return 1; }() == 1);
  REQUIRE([]() -> exco::result_t<float> { co_return -1.5; }() == -1.5f);
  REQUIRE([]() -> exco::result_t<int> {
    co_return exco::unerr(std::errc::address_in_use);
  }()
                          .error() == std::errc::address_in_use);
}
