#include <string>

namespace clanguml::t20072 {

bool validate(int v) { return v != 0; }

auto foo(int f)
{
    class Foo {
    public:
        void set(int value)
        {
            if (validate(value))
                value_ = value;
        }

        int get() const { return value_; }

    private:
        int value_;
    };

    Foo result;
    result.set(f);

    return result;
}

template <typename T> auto bar(const T &b, std::string msg = "two")
{
    struct Foo {
        int result(int c) const { return value / c; }

        int value;
    };

    Foo foo{b.get() + 10};

    return foo.result(msg.size());
}

int tmain()
{
    auto v = bar(foo(1));

    return 0;
}
} // namespace clanguml::t20072