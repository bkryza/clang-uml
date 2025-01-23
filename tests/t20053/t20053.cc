namespace clanguml {
namespace t20053 {

template <typename F> int a1(F &&f) { return f(42); }

int a2(int x) { return 2; }

int a3(int x) { return 3; }

int a4(int x)
{
    return []() { return 5; }();
}

int tmain()
{
    // Call expression in a nested lambda
    auto v1 = [](auto &&arg1) {
        return [](auto &&arg2) { return a2(arg2); }(arg1);
    }(0);

    // Nested lambda call without any actual calls
    auto v2 = [](auto &&arg1) {
        return [](auto &&arg2) { return arg2 + 2; }(arg1);
    }(0);

    // Call expression in a nested lambda in call expression
    // TODO: Fix double call to a3()
    auto v4 = a1(
        [](auto &&arg1) { return [](auto &&arg2) { return a3(arg2); }(arg1); });

    // Call to function returning value evaluated from a lambda expression
    auto v5 = a4(5);

    return 0;
}
}
}