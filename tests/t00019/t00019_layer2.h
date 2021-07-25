#pragma once

namespace clanguml {
namespace t00019 {

template <typename LowerLayer> class Layer2 : public LowerLayer {

    using LowerLayer::LowerLayer;

    using LowerLayer::m1;

    using LowerLayer::m2;

    int all_calls_count() const
    {
        return LowerLayer::m1_calls() + LowerLayer::m2_calls();
    }
};
}
}
