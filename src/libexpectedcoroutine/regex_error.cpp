module;

#include <expected>
#include <regex>
#include <system_error>

export module libexpectedcoroutine:regex_error;

export {
  namespace exco {
  struct regex_error_error_category : std::error_category {
    auto name() const noexcept -> char const * override;
    auto message(int ev) const -> std::string override;
  };
  } // namespace exco
}

namespace {
exco::regex_error_error_category const category{};
}

export {
  namespace exco {
  auto make_error_code(std::regex_constants::error_type error)
      -> std::error_code {
    return {static_cast<int>(error), category};
  }
  inline auto unerr(std::regex_constants::error_type const t) {
    return std::unexpected{make_error_code(t)};
  }
  } // namespace exco
  namespace std {
  template <>
  struct is_error_code_enum<std::regex_constants::error_type> : true_type {};
  } // namespace std
}

auto exco::regex_error_error_category::name() const noexcept -> char const * {
  return "std::regex_error";
}

auto exco::regex_error_error_category::message(int const ev) const
    -> std::string {
  switch (static_cast<std::regex_constants::error_type>(ev)) {
  default:
    return "";
  }
}
