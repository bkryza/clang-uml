#pragma once

#include <iostream>
#include <string>

namespace clanguml {
namespace t00019 {

template <typename LowerLayer> class Layer1 : public LowerLayer {

    using LowerLayer::LowerLayer;

    int m1() override
    {
        std::cout << "m1 called\n";
        return LowerLayer::m1();
    }

    std::string m2() override
    {
        std::cout << "m2 called\n";
        return LowerLayer::m2();
    }
};
}
}
