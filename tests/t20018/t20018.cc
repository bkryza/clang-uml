#include <iostream>

namespace clanguml {
namespace t20018 {

template <int N> struct Factorial {
    static const int value = N * Factorial<N - 1>::value;

    static void print() { Factorial<N - 1>::print(); }
};

template <> struct Factorial<0> {
    static const int value = 1;

    static void print() { std::cout << "Hello world\n"; }
};

void tmain() { Factorial<5>::print(); }
}
}