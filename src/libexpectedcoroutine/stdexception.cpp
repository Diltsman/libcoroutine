module;

#include <system_error>

export module libexpectedcoroutine:stdexception;

export {
  namespace exco {
  enum class stdexception {
    unknown = 1,
    exception,
    logic_error,
    invalid_argument,
    domain_error,
    length_error,
    out_of_range,
    // future_error, -- Use the internal error_code instead
    runtime_error,
    range_error,
    overflow_error,
    underflow_error,
    // regex_error, -- Use the internal error_code instead
    // system_error, -- Use the internal error_code instead
    ios_base_failure,
    // filesystem_error, -- Use the internal error_code instead
    tx_error,
    nonexistent_local_time,
    ambiguous_local_time,
    format_error,
    bad_typeid,
    bad_cast,
    bad_any_cast,
    bad_optional_access,
    bad_expected_access,
    bad_weak_ptr,
    bad_function_call,
    badd_alloc,
    bad_array_new_length,
    bad_exception,
    bad_variant_access
  };
  struct stdexception_error_category : std::error_category {
    auto name() const noexcept -> char const * override;
    auto message(int ev) const -> std::string override;
  };
  } // namespace exco
}

namespace {
exco::stdexception_error_category const category{};
}

export {
  namespace exco {
  auto make_error_code(stdexception exception) -> std::error_code {
    return {static_cast<int>(exception), category};
  }
  } // namespace exco
  namespace std {
  template <> struct is_error_code_enum<exco::stdexception> : true_type {};
  } // namespace std
}

auto exco::stdexception_error_category::name() const noexcept -> char const * {
  return "std::exception";
}

auto exco::stdexception_error_category::message(int const ev) const
    -> std::string {
  switch (static_cast<exco::stdexception>(ev)) {
  default:
    return "";
  }
}
