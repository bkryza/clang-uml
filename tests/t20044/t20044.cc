// #include "include/expected.hpp"

#include <functional>
#include <optional>
#include <thread>

namespace clanguml {
namespace t20044 {

enum class error { OK, FAIL };

namespace detail {
// Trivial std::expected mock-up just for testing calls through lambda
// expressions passed as arguments to methods
template <typename V, typename E> class expected {
private:
    std::optional<V> value_;
    std::optional<E> error_;

public:
    explicit expected(V v)
        : value_{std::move(v)}
    {
    }
    explicit expected(E e)
        : error_{std::move(e)}
    {
    }

    const auto &value() const { return *value_; }

    const auto &error() const { return *error_; }

    template <class F> auto and_then(F &&f) &&
    {
        if (value_)
            return f(*value_);

        return *this;
    }
};
} // namespace detail

namespace detail2 {
template <typename F> void run(F &&f) { f(); }
} // namespace detail2

using result_t = detail::expected<int, error>;

struct A {
    auto a() const { }

    auto a1() const { return result_t{10}; }

    auto a2(int arg) const { return result_t{arg + 1}; }

    auto a4(int arg) const { return result_t{arg + 1}; }

    void a5() { }
};

auto a3(int arg) { return result_t{arg + 1}; }

struct R {
    template <typename F> R(F &&f) { f(); }
};

int tmain()
{
    A a;

    // Call to template constructor with callable parameter and lambda
    // expression as argument
    R r([&a]() { a.a(); });

    std::function<result_t(const A &, int)> a4_wrapper = &A::a4;

    std::function<result_t(int)> a4_wrapper_to_a =
        std::bind(a4_wrapper, a, std::placeholders::_1);

    // The message to detail2::run() is skipped due to exclude filter, however
    // the call to lambda and A::a5() is rendered
    // TODO: Add some marker to highlight that this is not a direct call
    detail2::run([&]() { a.a5(); });

    return a
        .a1()
        // Call to a template method accepting a callable with lambda expression
        // as argument, fully tracked showing method's activity and
        .and_then([&](auto &&arg) { return a.a2(arg); })
        // TODO: Call to a method accepting a callable with function pointer
        //       as argument
        .and_then(a3)
        // TODO: Call to a method accepting a callable with std::function as
        //       argument
        .and_then(a4_wrapper_to_a)
        .value();
}
}
}