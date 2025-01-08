#include <iostream>
#include <type_traits>

/// Based on recursive-types-through-inheritance example from:
/// https://www.scs.stanford.edu/~dm/blog/param-pack.html

namespace clanguml::t00090 {

template <typename... T> struct HList;

template <> struct HList<> {
    static constexpr std::size_t len = 0;
};

template <typename T0, typename... TRest>
struct HList<T0, TRest...> : HList<TRest...> {
    using head_type = T0;
    using tail_type = HList<TRest...>;

    static constexpr std::size_t len = 1 + sizeof...(TRest);
    [[no_unique_address]] head_type value_{};

    constexpr HList() = default;
    template <typename U0, typename... URest>
    constexpr HList(U0 &&u0, URest &&...urest)
        : tail_type(std::forward<URest>(urest)...)
        , value_(std::forward<U0>(u0))
    {
    }

    head_type &head() & { return value_; }
    const head_type &head() const & { return value_; }
    head_type &&head() && { return value_; }

    tail_type &tail() & { return *this; }
    const tail_type &tail() const & { return *this; }
    tail_type &&tail() && { return *this; }
};

template <typename... T> HList(T...) -> HList<T...>;

template <typename T>
concept IsArithmetic = std::is_arithmetic_v<T>;

template <IsArithmetic... T> class Arithmetic : HList<T...> {
public:
    using HList<T...>::HList;

    constexpr double sum() const { return sumImpl(*this); }

private:
    template <typename L> static constexpr double sumImpl(const L &list)
    {
        if constexpr (L::len == 0) {
            return 0.0;
        }
        else {
            return static_cast<double>(list.head()) + sumImpl(list.tail());
        }
    }
};

int tmain()
{
    constexpr Arithmetic<int, float, double> a{11, 12.0, 13.0};

    return a.sum();
}
}