#pragma once

#include "t00018.h"

namespace clanguml {
namespace t00018 {
namespace impl {

class widget {
    int n;

public:
    void draw(const clanguml::t00018::widget &w) const;
    void draw(const clanguml::t00018::widget &w);
    widget(int n);
};
}
}
}
