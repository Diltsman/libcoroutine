import libexpectedcoroutine;

#include <catch2/catch_all.hpp>
#include <coroutine>

TEST_CASE("co_return returns expected value", "[expectedcoroutine]") {
  SECTION("co_return returns int") {
    auto const result = []() -> exco::result_t<int> { co_return 1; }();
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 1);
  }

  SECTION("co_return returns float") {
    auto const result = []() -> exco::result_t<float> { co_return -1.5; }();
    REQUIRE(result.has_value());
    REQUIRE(result.value() == -1.5f);
  }

  SECTION("co_return returns error_code") {
    auto const result = []() -> exco::result_t<int> {
      co_return exco::unerr(std::errc::address_in_use);
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == std::errc::address_in_use);
  }
}

TEST_CASE("co_await continues execution", "[expectedcoroutine]") {
  SECTION("single co_await") {
    auto const result = []() -> exco::result_t<int> {
      int x = co_await []() -> exco::result_t<int> { return 1; }();
      co_return x + 1;
    }();
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 2);
  }

  SECTION("multiple co_await") {
    auto const result = []() -> exco::result_t<int> {
      int x = co_await []() -> exco::result_t<int> { return 1; }();
      int y = co_await [](int a) -> exco::result_t<int> { return a + 1; }(x);
      co_return y + 1;
    }();
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 3);
  }
}

TEST_CASE("co_await return std::error_code", "[expectedcoroutine]") {
  SECTION("first co_await has error_code") {
    auto const result = []() -> exco::result_t<int> {
      int x = co_await []() -> exco::result_t<int> {
        return exco::unerr(std::errc::address_in_use);
      }();
      REQUIRE(false);
      co_return x + 1;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == std::errc::address_in_use);
  }

  SECTION("second co_await has error_code") {
    auto const result = []() -> exco::result_t<int> {
      int x = co_await []() -> exco::result_t<int> { return 1; }();
      REQUIRE(x == 1);
      int y = co_await [](int i) -> exco::result_t<int> {
        return exco::unerr(std::errc::address_in_use);
      }(x);
      REQUIRE(false);
      int z = co_await [](int i) -> exco::result_t<int> { return i + 1; }(y);
      co_return z + 1;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == std::errc::address_in_use);
  }
}

TEST_CASE("std::exceptions convert to error_code", "[expectedcoroutine]") {
  REQUIRE([]() -> exco::result_t<int> {
    throw std::invalid_argument{""};
    co_return 1;
  }()
                          .value() == 1);
}
