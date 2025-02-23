import libexpectedcoroutine;

#include <catch2/catch_all.hpp>
#include <chrono>
#include <coroutine>
#include <filesystem>
#include <future>
#include <regex>

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
  SECTION("exception before co_return") {
    auto const result = []() -> exco::result_t<int> {
      throw std::invalid_argument{""};
      co_return 1;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::invalid_argument).error());
  }

  SECTION("exception before co_await") {
    auto const result = []() -> exco::result_t<int> {
      throw std::invalid_argument{""};
      auto x = co_await []() -> exco::result_t<int> { return 1; }();
      co_return x;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::invalid_argument).error());
  }

  SECTION("exception after co_await") {
    auto const result = []() -> exco::result_t<int> {
      auto x = co_await []() -> exco::result_t<int> { return 1; }();
      throw std::invalid_argument{""};
      auto y = co_await [](int x) -> exco::result_t<int> { return x + 1; }(x);
      co_return y;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::invalid_argument).error());
  }
}

TEMPLATE_TEST_CASE_SIG(
    "std::exceptions convert to error_code", "[expectedcoroutine]",
    ((typename T, exco::stdexception V), T, V),
    (std::logic_error, exco::stdexception::logic_error),
    (std::invalid_argument, exco::stdexception::invalid_argument),
    (std::domain_error, exco::stdexception::domain_error),
    (std::length_error, exco::stdexception::length_error),
    (std::out_of_range, exco::stdexception::out_of_range),
    (std::range_error, exco::stdexception::range_error),
    (std::overflow_error, exco::stdexception::overflow_error),
    (std::underflow_error, exco::stdexception::underflow_error),
    (std::ios_base::failure, exco::stdexception::ios_base_failure)) {
  auto const result = []() -> exco::result_t<int> {
    throw T{""};
    co_return 0;
  }();
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == exco::unerr(V).error());
}

TEMPLATE_TEST_CASE_SIG("std::future_error convert to error_code",
                       "[expectedcoroutine]", ((std::future_errc V), V),
                       (std::future_errc::broken_promise),
                       (std::future_errc::future_already_retrieved),
                       (std::future_errc::promise_already_satisfied),
                       (std::future_errc::no_state)) {
  auto const result = []() -> exco::result_t<int> {
    throw std::future_error{V};
    co_return 0;
  }();
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == exco::unerr(V).error());
}

TEMPLATE_TEST_CASE_SIG(
    "std::regex_error convert to error_code", "[expectedcoroutine]",
    ((std::regex_constants::error_type V), V),
    (std::regex_constants::error_collate), (std::regex_constants::error_ctype),
    (std::regex_constants::error_escape), (std::regex_constants::error_backref),
    (std::regex_constants::error_brack), (std::regex_constants::error_paren),
    (std::regex_constants::error_brace), (std::regex_constants::error_badbrace),
    (std::regex_constants::error_range), (std::regex_constants::error_space),
    (std::regex_constants::error_badrepeat),
    (std::regex_constants::error_complexity),
    (std::regex_constants::error_stack)) {
  auto const result = []() -> exco::result_t<int> {
    throw std::regex_error{V};
    co_return 0;
  }();
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == exco::unerr(V).error());
}

TEST_CASE("thrown exceptions converted to correct error_code",
          "[expectedcoroutine]") {
  SECTION("std::exception") {
    auto const result = []() -> exco::result_t<int> {
      throw std::exception{};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::exception).error());
  }

  SECTION("std::filesystem::filesystem_error") {
    auto const result = []() -> exco::result_t<int> {
      throw std::filesystem::filesystem_error{
          "", exco::unerr(std::errc::address_in_use).error()};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == exco::unerr(std::errc::address_in_use).error());
  }

  SECTION("std::system_error") {
    auto const result = []() -> exco::result_t<int> {
      throw std::system_error{exco::unerr(std::errc::address_in_use).error()};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == exco::unerr(std::errc::address_in_use).error());
  }

#if defined(__cpp_transactional_memory) && __cpp_transactional_memory >= 201505
  SECTION("std::tx_exception") {
    auto const result = []() -> exco::result_t<int> {
      throw std::tx_exception{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::tx_exception).error());
  }
#endif

#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
  SECTION("std::chrono::nonexistent_local_time") {
    auto const result = []() -> exco::result_t<int> {
      throw std::chrono::nonexistent_local_time{
          std::chrono::current_zone()->to_local(
              std::chrono::system_clock::now())};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::tx_exception).error());
  }
#endif
}
