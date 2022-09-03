#pragma once

namespace clanguml {
namespace t00048 {

struct Base {
    int base;

    virtual void foo() = 0;
};

template <typename T> struct BaseTemplate {
    T base;

    virtual void foo() = 0;
};

}
}