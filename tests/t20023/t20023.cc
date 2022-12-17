#include <stdexcept>

namespace clanguml {
namespace t20023 {

struct A {
    int a1() { return 1; }
    int a2() { return 2; }
    int a3() { return 3; }
    int a4() { return 3; }

    int a()
    {
        try {
            return a1();
        }
        catch (std::runtime_error &) {
            return a2();
        }
        catch (std::logic_error &) {
            return a3();
        }
        catch (...) {
            return a4();
        }
    }
};

int tmain()
{
    A a;

    int result{};

    result = a.a();

    return result;
}
}
}