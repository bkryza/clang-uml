#pragma once

namespace clanguml {
namespace t20038 {

template <typename T> T add_impl(T a, T b) { return a + b; };

template <typename T> T add(T a, T b)
{
    // Invoke 'add' implementation
    return add_impl(a, b);
};
}
}