#pragma once

#include <experimental/propagate_const>
#include <iostream>
#include <memory>

namespace clanguml {
namespace t00018 {

namespace impl {
class widget;
}

// Pimpl example based on https://en.cppreference.com/w/cpp/language/pimpl
class widget {
    std::unique_ptr<impl::widget> pImpl;

public:
    void draw() const;
    void draw();
    bool shown() const { return true; }
    widget(int);
    ~widget();

    widget(widget &&);

    widget(const widget &) = delete;
    widget &operator=(widget &&);
    widget &operator=(const widget &) = delete;
};
}
}
