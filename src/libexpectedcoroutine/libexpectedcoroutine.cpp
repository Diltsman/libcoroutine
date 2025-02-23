module;

#include <any>
#include <coroutine>
#include <expected>
#include <filesystem>
#include <functional>
#include <future>
#include <optional>
#include <regex>
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <variant>

export module libexpectedcoroutine;
export import :stdexception;
export import :regex_error;

export {
  namespace exco {
  template <typename T> using result_t = std::expected<T, std::error_code>;

  inline auto unerr(auto const t) {
    return std::unexpected{make_error_code(t)};
  }

  template <typename T> struct expected_wrapper {
    // Initialize as errno 0 so there are no restrictions on T
    // caused by initializing this
    exco::result_t<T> m_result{exco::unerr(static_cast<std::errc>(0))};
    expected_wrapper<T> *&m_ptr_to_this;
    expected_wrapper(expected_wrapper<T> *&ptr_to_this)
        : m_ptr_to_this{ptr_to_this} {
      m_ptr_to_this = this;
    }
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
      exco::expected_wrapper<T> *m_ptr_to_wrapper;
      auto initial_suspend() noexcept -> std::suspend_never { return {}; }
      auto final_suspend() noexcept -> std::suspend_never { return {}; }
      auto return_value(std::error_code ec) {
        m_ptr_to_wrapper->m_result = std::unexpected{ec};
      }
      auto return_value(auto &&t) { m_ptr_to_wrapper->m_result = std::move(t); }
      auto get_return_object() {
        return exco::expected_wrapper<T>{m_ptr_to_wrapper};
      }
      auto unhandled_exception() {
        try {
          throw;
        } catch (std::invalid_argument const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::invalid_argument);
        } catch (std::domain_error const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::domain_error);
        } catch (std::length_error const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::length_error);
        } catch (std::out_of_range const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::out_of_range);
        } catch (std::future_error const &e) {
          m_ptr_to_wrapper->m_result = std::unexpected{e.code()};
        } catch (std::logic_error const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::logic_error);
        } catch (std::range_error const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::range_error);
        } catch (std::overflow_error const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::overflow_error);
        } catch (std::underflow_error const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::underflow_error);
        } catch (std::regex_error const &e) {
          m_ptr_to_wrapper->m_result = exco::unerr(e.code());
        } catch (std::ios_base::failure const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::ios_base_failure);
        } catch (std::filesystem::filesystem_error const &e) {
          m_ptr_to_wrapper->m_result = std::unexpected{e.code()};
        } catch (std::system_error const &e) {
          m_ptr_to_wrapper->m_result = std::unexpected{e.code()};
#if defined(__cpp_transactional_memory) && __cpp_transactional_memory >= 201505
        } catch (std::tx_exception<void> const &) {
          static_assert(false, "tx_exception is a template, how to handle it?");
#endif
#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
          static_assert(
              false, "std::chrono::nonexistent_local_time && "
                     "std::chrono::ambiguous_local_time not implemented yet");
#endif
        } catch (std::format_error const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::format_error);
        } catch (std::runtime_error const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::runtime_error);
        } catch (std::bad_typeid const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_typeid);
        } catch (std::bad_any_cast const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_any_cast);
        } catch (std::bad_cast const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_cast);
        } catch (std::bad_optional_access const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_optional_access);
        } catch (std::bad_expected_access<void> const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_expected_access);
        } catch (std::bad_weak_ptr const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_weak_ptr);
        } catch (std::bad_function_call const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_function_call);
        } catch (std::bad_array_new_length const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_array_new_length);
        } catch (std::bad_alloc const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_alloc);
        } catch (std::bad_exception const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_exception);
        } catch (std::bad_variant_access const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::bad_variant_access);
        } catch (std::exception const &) {
          m_ptr_to_wrapper->m_result =
              exco::unerr(exco::stdexception::exception);
        } catch (...) {
          m_ptr_to_wrapper->m_result = exco::unerr(exco::stdexception::unknown);
        }
      }
      template <typename T1> auto await_transform(exco::result_t<T1> value) {
        m_ptr_to_wrapper->m_result = std::move(value);
        return awaiter_type<T1>{m_ptr_to_wrapper->m_result};
      }
    };
  };
  } // namespace std
}
