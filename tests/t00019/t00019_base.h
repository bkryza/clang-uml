#pragma once

#include <string>

namespace clanguml {
namespace t00019 {

class Base {

    Base() = default;

    virtual ~Base() = default;

    virtual int m1() { return 2; }

    virtual std::string m2() { return "two"; }
};
}
}
