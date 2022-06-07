// Inspired by skypjack/entt signal handlers
// This test case checks that at least clang-uml does not crash on this code
namespace clanguml::t00044 {

template <typename T> class sink;

template <typename T, typename A> class signal_handler;

template <typename Ret, typename... Args, typename A>
class sink<signal_handler<Ret(Args...), A>> {
    using signal_t = signal_handler<Ret(Args...), A>;

public:
    sink(signal_t &sh)
        : signal{&sh}
    {
    }

private:
    signal_t *signal;
};

template <typename Ret, typename... Args, typename A>
class signal_handler<Ret(Args...), A> {
};

template <typename Ret, typename... Args, typename A>
sink(signal_handler<Ret(Args...), A> &)
    -> sink<signal_handler<Ret(Args...), A>>;

signal_handler<void(int), bool> int_handler;

sink sink1{int_handler};

} // namespace clanguml::t00044
