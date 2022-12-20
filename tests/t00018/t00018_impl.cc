#include "t00018_impl.h"
#include "t00018.h"

namespace clanguml {
namespace t00018 {
namespace impl {

widget::widget(int n)
    : n(n)
{
}

void widget::draw(const clanguml::t00018::widget &w) const
{
    if (w.shown())
        std::cout << "drawing a const widget " << n << '\n';
}

void widget::draw(const clanguml::t00018::widget &w)
{
    if (w.shown())
        std::cout << "drawing a non-const widget " << n << '\n';
}
} // namespace impl
} // namespace t00018
} // namespace clanguml
