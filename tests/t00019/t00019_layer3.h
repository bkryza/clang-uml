#pragma once

#include <string>

namespace clanguml {
namespace t00019 {

template <typename LowerLayer> class Layer3 : public LowerLayer {

    using LowerLayer::LowerLayer;

    virtual int m1() override
    {
        m_m1_calls++;
        return LowerLayer::m1();
    }

    virtual std::string m2() override
    {
        m_m2_calls++;
        return LowerLayer::m2();
    }

    int m1_calls() const { return m_m1_calls; }

    int m2_calls() const { return m_m2_calls; }

private:
    int m_m1_calls{};
    int m_m2_calls{};
};
}
}
