module;

#include <coroutine>
#include <expected>
#include <system_error>

export module libexpectedcoroutine;

export {
  namespace exco {
  template <typename T> using result_t = std::expected<T, std::error_code>;

  template <typename T> inline auto unerr(T const t) {
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
    struct promise_type {
      exco::result_t<T> m_result{1};
      auto initial_suspend() noexcept -> std::suspend_never { return {}; }
      auto final_suspend() noexcept -> std::suspend_never { return {}; }
      auto return_value(std::error_code ec) { m_result = std::unexpected{ec}; }
      template <typename T1> auto return_value(T1 &&t) {
        m_result = std::move(t);
      }
      auto get_return_object() { return exco::expected_wrapper<T>{m_result}; }
      auto unhandled_exception() {}
    };
  };
  } // namespace std
}
