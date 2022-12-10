#include <iostream>

namespace clanguml {
namespace t20018 {

template <int N> struct Factorial {
    static const int value = N * Factorial<N - 1>::value;

    static void print(int answer) { Factorial<N - 1>::print(answer); }
};

template <> struct Factorial<0> {
    static const int value = 1;

    static void print(int answer)
    {
        std::cout << "The answer is " << answer << "\n";
    }
};

template <typename T, int N = T::value> struct Answer {
    static void print() { T::print(N); }
};

void tmain() { Answer<Factorial<5>>::print(); }
}
}