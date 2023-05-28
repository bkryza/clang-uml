#pragma once

#include "concepts/concepts.h"

namespace clanguml {
namespace t00065 {
struct B {
    B() = default;
    void b() { }
};

template <typename T> struct C {
    T *t;
};

template <bconcept T> struct D {
    T t;
    C<int> c;
};

}
}