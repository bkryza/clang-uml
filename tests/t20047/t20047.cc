#include <future>

namespace clanguml {
namespace t20047 {

int a1(int x) { return x + 1; }

int a2(int x) { return x + 2; }

int a3(int x) { return x + 3; }

int a4(int x) { return x + 4; }

int a5(int x) { return x + 5; }

int a6(int x) { return x + 6; }

int run(int (*f)(int), int arg) { return f(arg); }

int tmain()
{
    auto res =
        // \uml{call clanguml::t20047::a1(int)}
        run(a1, 0);

    res = a3(
        // \uml{call clanguml::t20047::a2(int)}
        run(a2, 0));

    // \uml{call clanguml::t20047::a4(int)}
    res = [](auto &&x) { return a4(x); }(0);

    // \uml{call clanguml::t20047::a5(int)}
    res = std::async(a5, 10).get();

    // \uml{call clanguml::t20047::a6(int)}
    res = [](auto &&x) { return std::async(run, a6, x).get(); }(1);

    return res;
}
}
}