#include <optional>
#include <string>
#include <vector>

namespace clanguml::t20073 {

template <typename T> auto maybe(const std::optional<T> &v) -> T
{
    return v.has_value() ? *v : T{};
}

double add(int x, float y, std::optional<double> z) { return x + y + maybe(z); }

struct A {
    void set(double v) { a = v; }

    double a;
};

int tmain(std::vector<std::string> args)
{
    A a;

    a.set(add(1, 2.0, std::make_optional<double>(3.0)));

    return 0;
}
}
