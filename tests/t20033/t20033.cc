#include <cmath>
#include <cstdint>
#include <vector>

namespace clanguml {
namespace t20033 {
struct A {
    int a1() { return 0; }
    int a2() { return 1; }
    int a3() { return 2; }
    int a4() { return 3; }
};

int tmain()
{
    A a;

    int result{};
    // clang-format off
    if(false) {
        result = 0;
    }
    else if (reinterpret_cast<uint64_t>(&a) % 100 == 0ULL) {
        result = a.a1();
    }
    else if (reinterpret_cast<uint64_t>(&a) % 64 == 0ULL) {
        result = a.a2();
    }
    else if(a.a2() == 2 &&
            a.a3() == 3) {
        result = a.a3();
    }
    else {
        result = a.a4();
    }
    // clang-format on

    if (int i = a.a2(); i != 2) {
        result += a.a3();
    }

    for (int i = 0; i < a.a2(); i++) {
        result += i * a.a3();
    }

    int retry_count = a.a3();
    while (retry_count--) {
        result -= a.a2();
    }

    do {
        result += a.a4();
    } while (retry_count++ < a.a3());

    result = a.a4() % 6 ? result * 2 : result;

    std::vector<int> ints;
    for (auto i : ints) {
        result += a.a4();
    }

    return result;
}
}
}