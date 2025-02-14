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

TEST_CASE("co_await continues execution", "[expectedcoroutine]") {
  REQUIRE([]() -> exco::result_t<int> {
    int x = co_await []() -> exco::result_t<int> { return 1; }();
    co_return x + 1;
  }()
                          .value() == 2);
  REQUIRE([]() -> exco::result_t<int> {
    int x = co_await []() -> exco::result_t<int> { return 1; }();
    int y = co_await [](int a) -> exco::result_t<int> { return a + 1; }(x);
    co_return y + 1;
  }()
                          .value() == 3);
}

TEST_CASE("co_await return std::error_code", "[expectedcoroutine]") {
  REQUIRE([]() -> exco::result_t<int> {
    int x = co_await []() -> exco::result_t<int> {
      return exco::unerr(std::errc::address_in_use);
    }();
    REQUIRE(false);
    co_return x + 1;
  }()
                          .error() == std::errc::address_in_use);
  REQUIRE([]() -> exco::result_t<int> {
    int x = co_await []() -> exco::result_t<int> { return 1; }();
    REQUIRE(x == 1);
    int y = co_await [](int i) -> exco::result_t<int> {
      return exco::unerr(std::errc::address_in_use);
    }(x);
    REQUIRE(false);
    int z = co_await [](int i) -> exco::result_t<int> { return i + 1; }(y);
    co_return z + 1;
  }()
                          .error() == std::errc::address_in_use);
}
