namespace clanguml::t20069 {

int foo(int x) { return x * 2; }

int bar(int x) { return x / 2; }

template <typename T> T baz(T x) { return foo(x) + bar(x); }

struct A {
    int a(int x)
    {
        if (x % 2 == 0) {
            return x;
        }
        else if (x > 2) {
            return bar(x);
        }
        else if (x == 123) {
            auto f = [](auto &v) { return v + 1; };

            return f(x);
        }
        else {
            return 0;
        }
    }
};

int tmain()
{
    A a;

    auto x = a.a(100);
    if (x > 100) {
        return x;
    }
    else if (x == 200) {
        return baz(x);
    }

    return 0;
}
} // namespace clanguml::t20069