namespace clanguml {
namespace t20046 {

template <typename F> int a1(F &&f) { return f(42); }

int a2(int x) { return 2; }

int a3(int x) { return 3; }

int tmain()
{
    // Call expression in a nested lambda
    auto v1 = [](auto &&arg1) {
        return [](auto &&arg2) { return a2(arg2); }(arg1);
    }(0);

    // Call expression in a nested lambda in call expression
    auto v4 = a1(
        [](auto &&arg1) { return [](auto &&arg2) { return a3(arg2); }(arg1); });

    return 0;
}
}
}