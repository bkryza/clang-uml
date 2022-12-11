#pragma once

namespace clanguml {
namespace t20017 {
int b1(int x, int y);

template <typename T> T b2(T x, T y) { return x / y; }
}
}