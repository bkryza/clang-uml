#include "generator.h"

#include "class_diagram/model/jinja_context.h"
#include "include_diagram/model/jinja_context.h"
#include "package_diagram/model/jinja_context.h"
#include "sequence_diagram/model/jinja_context.h"

namespace clanguml::common::generators {

template <typename C, typename D> void generator<C, D>::update_context() const
{
    m_context["diagram"] = common::jinja::diagram_context<D>(model());
}

//
// Explicitly instantiate update_context() for all diagram types
//
template void generator<clanguml::config::class_diagram,
    clanguml::class_diagram::model::diagram>::update_context() const;
template void generator<clanguml::config::sequence_diagram,
    clanguml::sequence_diagram::model::diagram>::update_context() const;
template void generator<clanguml::config::include_diagram,
    clanguml::include_diagram::model::diagram>::update_context() const;
template void generator<clanguml::config::package_diagram,
    clanguml::package_diagram::model::diagram>::update_context() const;

} // namespace clanguml::common::generators