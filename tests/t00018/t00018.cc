#include "t00018.h"
#include "t00018_impl.h"

namespace clanguml {
namespace t00018 {

void widget::draw() const { pImpl->draw(*this); }

void widget::draw() { pImpl->draw(*this); }

widget::widget(int n)
    : pImpl{std::make_unique<impl::widget>(n)}
{
}

widget::widget(widget &&) = default;

widget::~widget() = default;

widget &widget::operator=(widget &&) = default;
} // namespace t00018
} // namespace clanguml
