module;

#include <coroutine>
#include <expected>
#include <system_error>

export module libexpectedcoroutine;

export {
  namespace exco {
  template <typename T> using result_t = std::expected<T, std::error_code>;

  inline auto unerr(auto const t) {
    return std::unexpected{make_error_code(t)};
  }

  template <typename T> struct expected_wrapper {
    result_t<T> &m_result;
    operator result_t<T>() { return std::move(m_result); };
  };
  } // namespace exco

  namespace std {
  template <typename T, typename... Args>
  struct coroutine_traits<exco::result_t<T>, Args...> {
    struct promise_type;
    template <typename T1> struct awaiter_type {
      exco::result_t<T1> &m_result;
      explicit awaiter_type(exco::result_t<T1> &result) noexcept
          : m_result{result} {}
      auto await_ready() { return m_result.has_value(); }
      auto await_suspend(std::coroutine_handle<promise_type> h) {
        // This should only happen when await_ready() returns false,
        // which means that has_value() returned false.
        h.destroy();
      }
      auto await_resume() {
        // This should only happen when await_ready() returns true,
        // which means that has_value() returned true.
        return m_result.value();
      }
    };

    struct promise_type {
      // Initialize as errno 0 so there are no restrictions on T
      // caused by initializing this
      exco::result_t<T> m_result{exco::unerr(static_cast<std::errc>(0))};
      auto initial_suspend() noexcept -> std::suspend_never { return {}; }
      auto final_suspend() noexcept -> std::suspend_never { return {}; }
      auto return_value(std::error_code ec) { m_result = std::unexpected{ec}; }
      auto return_value(auto &&t) { m_result = std::move(t); }
      auto get_return_object() { return exco::expected_wrapper<T>{m_result}; }
      auto unhandled_exception() {}
      template <typename T1> auto await_transform(exco::result_t<T1> value) {
        m_result = value;
        return awaiter_type<T1>{value};
      }
    };
  };
  } // namespace std
}
