#include "t00048.h"

#pragma once

namespace clanguml {
namespace t00048 {

struct B : public Base {
    int b;

    void foo() override;
};

template <typename T> struct BTemplate : public BaseTemplate<T> {
    T b;

    void foo() override { }
};

}
}