#include <future>

namespace clanguml {
namespace t20048 {

int a1(int x) { return x + 1; }

int a2(int x) { return x + 2; }

int a3(int x) { return x + 3; }

int a4(int x) { return x + 4; }

int a5(int x) { return x + 5; }

int a6(int x) { return x + 6; }

int a7(int x) { return x + 7; }

int tmain()
{
    // a1() adds `1` to the result of a2()
    auto res = a1(a2(a3(0)));

    // This lambda calls a4() which adds `4` to it's argument
    res = [](auto &&x) { return a4(x); }(0);

    // a5() adds `1` to the result of a6()
    res = a5(
        // a6() adds `1` to its argument
        a6(0));

    // a7() is called via add std::async
    // \uml{call clanguml::t20048::a7(int)}
    res = std::async(a7, 10).get();

    return 0;
}
}
}