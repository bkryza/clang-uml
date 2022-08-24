#include "t00048.h"

#pragma once

namespace clanguml {
namespace t00048 {

struct A : public Base {
    int a;

    void foo() override;
};

template <typename T> struct ATemplate : public BaseTemplate<T> {
    T a;

    void foo() override { }
};

}
}