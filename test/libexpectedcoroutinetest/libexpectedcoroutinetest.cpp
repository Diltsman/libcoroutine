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

  SECTION("std::logic_error") {
    auto const result = []() -> exco::result_t<int> {
      throw std::logic_error{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::logic_error).error());
  }

  SECTION("std::invalid_argument") {
    auto const result = []() -> exco::result_t<int> {
      throw std::invalid_argument{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::invalid_argument).error());
  }

  SECTION("std::domain_error") {
    auto const result = []() -> exco::result_t<int> {
      throw std::domain_error{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::domain_error).error());
  }

  SECTION("std::length_error") {
    auto const result = []() -> exco::result_t<int> {
      throw std::length_error{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::length_error).error());
  }

  SECTION("std::out_of_range") {
    auto const result = []() -> exco::result_t<int> {
      throw std::out_of_range{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::out_of_range).error());
  }

  SECTION("std::future_error") {
    SECTION("broken_promise") {
      auto const result = []() -> exco::result_t<int> {
        throw std::future_error{std::future_errc::broken_promise};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::future_errc::broken_promise).error());
    }

    SECTION("future_already_retrieved") {
      auto const result = []() -> exco::result_t<int> {
        throw std::future_error{std::future_errc::future_already_retrieved};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::future_errc::future_already_retrieved).error());
    }

    SECTION("promise_already_satisfied") {
      auto const result = []() -> exco::result_t<int> {
        throw std::future_error{std::future_errc::promise_already_satisfied};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::future_errc::promise_already_satisfied).error());
    }

    SECTION("no_state") {
      auto const result = []() -> exco::result_t<int> {
        throw std::future_error{std::future_errc::no_state};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::future_errc::no_state).error());
    }
  }

  SECTION("std::range_error") {
    auto const result = []() -> exco::result_t<int> {
      throw std::range_error{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::range_error).error());
  }

  SECTION("std::overflow_error") {
    auto const result = []() -> exco::result_t<int> {
      throw std::overflow_error{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::overflow_error).error());
  }

  SECTION("std::underflow_error") {
    auto const result = []() -> exco::result_t<int> {
      throw std::underflow_error{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::underflow_error).error());
  }

  SECTION("std::regex_error") {
    SECTION("error_collate") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_collate};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_collate).error());
    }

    SECTION("error_ctype") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_ctype};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_ctype).error());
    }

    SECTION("error_escape") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_escape};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_escape).error());
    }

    SECTION("error_backref") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_backref};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_backref).error());
    }

    SECTION("error_brack") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_brack};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_brack).error());
    }

    SECTION("error_paren") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_paren};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_paren).error());
    }

    SECTION("error_brace") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_brace};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_brace).error());
    }

    SECTION("error_badbrace") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_badbrace};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_badbrace).error());
    }

    SECTION("error_range") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_range};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_range).error());
    }

    SECTION("error_space") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_space};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_space).error());
    }

    SECTION("error_badrepeat") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_badrepeat};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_badrepeat).error());
    }

    SECTION("error_complexity") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_complexity};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_complexity).error());
    }

    SECTION("error_stack") {
      auto const result = []() -> exco::result_t<int> {
        throw std::regex_error{std::regex_constants::error_stack};
        co_return 0;
      }();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() ==
              exco::unerr(std::regex_constants::error_stack).error());
    }
  }

  SECTION("std::ios_base::failure") {
    auto const result = []() -> exco::result_t<int> {
      throw std::ios_base::failure{""};
      co_return 0;
    }();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            exco::unerr(exco::stdexception::ios_base_failure).error());
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
