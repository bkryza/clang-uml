#include "submodule1a/submodule1a.h"

#pragma once

namespace clanguml {
namespace t00065 {

enum class ABC { a, b, c };

enum XYZ { x, y, z };

struct A {
    ABC abc;
    XYZ xyz;
    detail::AImpl *pimpl;
};
}
}