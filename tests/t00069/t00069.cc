#include <coroutine>
#include <optional>

namespace clanguml {
namespace t00069 {

template <typename T> struct generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    generator(handle_type h)
        : h_(h)
    {
    }

    ~generator() { h_.destroy(); }

    struct promise_type {
        T value_;
        std::exception_ptr exception_;

        generator get_return_object()
        {
            return generator(handle_type::from_promise(*this));
        }
        std::suspend_always initial_suspend() { return {}; }

        std::suspend_always final_suspend() noexcept { return {}; }

        void unhandled_exception() { exception_ = std::current_exception(); }

        template <std::convertible_to<T> From>
        std::suspend_always yield_value(From &&from)
        {
            value_ = std::forward<From>(from);
            return {};
        }

        void return_void() { }
    };

    handle_type h_;

private:
    bool full_ = false;
};

class A {
public:
    generator<unsigned long> iota() { co_yield counter_++; }

    generator<unsigned long> seed()
    {
        counter_ = 42;
        co_return;
    }

private:
    unsigned long counter_;
};

} // namespace t00069
} // namespace clanguml
