#pragma once

namespace clanguml {
namespace t20014 {

template <typename T, typename F> struct C {
    F c1(F i, F j) { return c_.b1(i, j); }

    F c2(F i, F j) { return c_.b2(i, j); }

    T c_;
};

}
}