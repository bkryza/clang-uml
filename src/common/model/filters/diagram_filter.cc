/**
 * @file src/common/model/diagram_filter.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diagram_filter.h"

#include <utility>

#include "class_diagram/model/class.h"
#include "common/model/package.h"
#include "glob/glob.hpp"
#include "include_diagram/model/diagram.h"
#include "package_diagram/model/diagram.h"
#include "sequence_diagram/model/diagram.h"

namespace clanguml::common::model {

namespace detail {

template <>
const clanguml::common::reference_vector<class_diagram::model::class_> &view(
    const class_diagram::model::diagram &d)
{
    return d.classes();
}

template <>
const clanguml::common::reference_vector<class_diagram::model::enum_> &view(
    const class_diagram::model::diagram &d)
{
    return d.enums();
}

template <>
const clanguml::common::reference_vector<common::model::package> &view(
    const package_diagram::model::diagram &d)
{
    return d.packages();
}

template <>
const clanguml::common::reference_vector<common::model::source_file> &view(
    const include_diagram::model::diagram &d)
{
    return d.files();
}

template <>
const clanguml::common::optional_ref<class_diagram::model::class_> get(
    const class_diagram::model::diagram &d, const std::string &full_name)
{
    return d.find<class_diagram::model::class_>(full_name);
}

template <>
const clanguml::common::optional_ref<common::model::package> get(
    const package_diagram::model::diagram &d, const std::string &full_name)
{
    return d.find<package>(full_name);
}

template <>
const clanguml::common::optional_ref<common::model::source_file> get(
    const include_diagram::model::diagram &d, const std::string &full_name)
{
    return d.find<source_file>(full_name);
}

template <>
eid_t destination_comparator<common::model::source_file>(
    const common::model::source_file &f)
{
    return f.id();
}
} // namespace detail

filter_visitor::filter_visitor(filter_t type)
    : type_{type}
{
}

tvl::value_t filter_visitor::match(
    const diagram & /*d*/, const common::model::element & /*e*/) const
{
    return {};
}

tvl::value_t filter_visitor::match(
    const diagram &d, const common::model::relationship &r) const
{
    return match(d, r.type());
}

tvl::value_t filter_visitor::match(
    const diagram & /*d*/, const common::model::relationship_t & /*r*/) const
{
    return {};
}

tvl::value_t filter_visitor::match(
    const diagram & /*d*/, const common::model::access_t & /*a*/) const
{
    return {};
}

tvl::value_t filter_visitor::match(
    const diagram & /*d*/, const common::model::namespace_ & /*ns*/) const
{
    return {};
}

tvl::value_t filter_visitor::match(
    const diagram & /*d*/, const common::model::source_file & /*f*/) const
{
    return {};
}

tvl::value_t filter_visitor::match(
    const diagram & /*d*/, const common::model::source_location & /*f*/) const
{
    return {};
}

tvl::value_t filter_visitor::match(
    const diagram &d, const class_diagram::model::class_method &m) const
{
    return match(d, m.access());
}

tvl::value_t filter_visitor::match(
    const diagram &d, const class_diagram::model::class_member &m) const
{
    return match(d, m.access());
}

tvl::value_t filter_visitor::match(const diagram & /*d*/,
    const sequence_diagram::model::participant & /*p*/) const
{
    return {};
}

bool filter_visitor::is_inclusive() const
{
    return type_ == filter_t::kInclusive;
}

bool filter_visitor::is_exclusive() const
{
    return type_ == filter_t::kExclusive;
}

filter_t filter_visitor::type() const { return type_; }

filter_mode_t filter_visitor::mode() const { return mode_; }

void filter_visitor::set_mode(filter_mode_t mode) { mode_ = mode; }

anyof_filter::anyof_filter(
    filter_t type, std::vector<std::unique_ptr<filter_visitor>> filters)
    : filter_visitor{type}
    , filters_{std::move(filters)}
{
}

tvl::value_t anyof_filter::match(
    const diagram &d, const common::model::element &e) const
{
    return match_anyof(d, e);
}

tvl::value_t anyof_filter::match(
    const diagram &d, const common::model::relationship_t &r) const
{
    return match_anyof(d, r);
}

tvl::value_t anyof_filter::match(
    const diagram &d, const common::model::access_t &a) const
{
    return match_anyof(d, a);
}

tvl::value_t anyof_filter::match(
    const diagram &d, const common::model::namespace_ &ns) const
{
    return match_anyof(d, ns);
}

tvl::value_t anyof_filter::match(
    const diagram &d, const common::model::source_file &f) const
{
    return match_anyof(d, f);
}

tvl::value_t anyof_filter::match(
    const diagram &d, const common::model::source_location &f) const
{
    return match_anyof(d, f);
}

tvl::value_t anyof_filter::match(
    const diagram &d, const class_diagram::model::class_method &m) const
{
    return match_anyof(d, m);
}

tvl::value_t anyof_filter::match(
    const diagram &d, const class_diagram::model::class_member &m) const
{
    return match_anyof(d, m);
}

tvl::value_t anyof_filter::match(
    const diagram &d, const sequence_diagram::model::participant &p) const
{
    return match_anyof(d, p);
}

allof_filter::allof_filter(
    filter_t type, std::vector<std::unique_ptr<filter_visitor>> filters)
    : filter_visitor{type}
    , filters_{std::move(filters)}
{
}

tvl::value_t allof_filter::match(
    const diagram &d, const common::model::element &e) const
{
    return match_allof(d, e);
}

tvl::value_t allof_filter::match(
    const diagram &d, const common::model::relationship_t &r) const
{
    return match_allof(d, r);
}

tvl::value_t allof_filter::match(
    const diagram &d, const common::model::access_t &a) const
{
    return match_allof(d, a);
}

tvl::value_t allof_filter::match(
    const diagram &d, const common::model::namespace_ &ns) const
{
    return match_allof(d, ns);
}

tvl::value_t allof_filter::match(
    const diagram &d, const common::model::source_file &f) const
{
    return match_allof(d, f);
}

tvl::value_t allof_filter::match(
    const diagram &d, const common::model::source_location &f) const
{
    return match_allof(d, f);
}

tvl::value_t allof_filter::match(
    const diagram &d, const class_diagram::model::class_method &m) const
{
    return match_allof(d, m);
}

tvl::value_t allof_filter::match(
    const diagram &d, const class_diagram::model::class_member &m) const
{
    return match_allof(d, m);
}

tvl::value_t allof_filter::match(
    const diagram &d, const sequence_diagram::model::participant &p) const
{
    return match_allof(d, p);
}

namespace_filter::namespace_filter(
    filter_t type, std::vector<common::namespace_or_regex> namespaces)
    : filter_visitor{type}
    , namespaces_{std::move(namespaces)}
{
}

tvl::value_t namespace_filter::match(
    const diagram & /*d*/, const namespace_ &ns) const
{
    if (ns.is_empty())
        return {};

    return tvl::any_of(namespaces_.begin(), namespaces_.end(),
        [&ns, is_inclusive = is_inclusive()](const auto &nsit) {
            if (std::holds_alternative<namespace_>(nsit.value())) {
                const auto &ns_pattern = std::get<namespace_>(nsit.value());
                if (is_inclusive)
                    return ns.starts_with(ns_pattern) ||
                        ns_pattern.starts_with(ns);

                return ns.starts_with(ns_pattern);
            }

            const auto &regex = std::get<common::regex>(nsit.value());
            return regex %= ns.to_string();
        });
}

tvl::value_t namespace_filter::match(const diagram &d, const element &e) const
{
    if (d.type() != diagram_t::kPackage &&
        dynamic_cast<const package *>(&e) != nullptr) {
        auto result = tvl::any_of(namespaces_.begin(), namespaces_.end(),
            [&e, is_inclusive = is_inclusive()](const auto &nsit) {
                if (std::holds_alternative<namespace_>(nsit.value())) {
                    const auto &ns_pattern = std::get<namespace_>(nsit.value());

                    auto element_full_name_starts_with_namespace =
                        namespace_{e.name_and_ns()}.starts_with(ns_pattern);

                    auto element_full_name_equals_pattern =
                        namespace_{e.name_and_ns()} == ns_pattern;

                    auto pattern_starts_with_element_full_name =
                        ns_pattern.starts_with(namespace_{e.name_and_ns()});

                    auto result = element_full_name_starts_with_namespace ||
                        element_full_name_equals_pattern;

                    if (is_inclusive)
                        result =
                            result || pattern_starts_with_element_full_name;

                    return result;
                }

                return std::get<common::regex>(nsit.value()) %=
                    e.full_name(false);
            });

        return result;
    }

    if (d.type() == diagram_t::kPackage) {
        auto result = tvl::any_of(namespaces_.begin(), namespaces_.end(),
            [&e, is_inclusive = is_inclusive()](const auto &nsit) {
                if (std::holds_alternative<namespace_>(nsit.value())) {
                    auto e_ns = namespace_{e.full_name(false)};
                    auto nsit_ns = std::get<namespace_>(nsit.value());

                    if (is_inclusive)
                        return e_ns.starts_with(nsit_ns) ||
                            nsit_ns.starts_with(e_ns) || e_ns == nsit_ns;

                    return e_ns.starts_with(nsit_ns) || e_ns == nsit_ns;
                }

                return std::get<common::regex>(nsit.value()) %=
                    e.full_name(false);
            });

        return result;
    }

    auto result = tvl::any_of(
        namespaces_.begin(), namespaces_.end(), [&e](const auto &nsit) {
            if (std::holds_alternative<namespace_>(nsit.value())) {
                return e.get_namespace().starts_with(
                    std::get<namespace_>(nsit.value()));
            }

            return std::get<common::regex>(nsit.value()) %= e.full_name(false);
        });

    return result;
}

tvl::value_t namespace_filter::match(
    const diagram &d, const sequence_diagram::model::participant &p) const
{
    return match(d, dynamic_cast<const element &>(p));
}

modules_filter::modules_filter(
    filter_t type, std::vector<common::string_or_regex> modules)
    : filter_visitor{type}
    , modules_{std::move(modules)}
{
}

tvl::value_t modules_filter::match(
    const diagram & /*d*/, const element &e) const
{
    if (modules_.empty())
        return {};

    if (!e.module().has_value())
        return {false};

    auto module_toks =
        path::split(e.module().value(), path_type::kModule); // NOLINT

    if (dynamic_cast<const package *>(&e) != nullptr &&
        e.get_namespace().type() == path_type::kModule) {
        module_toks.push_back(e.name());
    }

    auto result = tvl::any_of(modules_.begin(), modules_.end(),
        [&e, &module_toks](const auto &modit) {
            if (std::holds_alternative<std::string>(modit.value())) {
                const auto &modit_str = std::get<std::string>(modit.value());
                const auto modit_toks =
                    path::split(modit_str, path_type::kModule);

                return e.module() == modit_str ||
                    util::starts_with(module_toks, modit_toks);
            }

            return std::get<common::regex>(modit.value()) %= e.module().value();
        });

    return result;
}

element_filter::element_filter(
    filter_t type, std::vector<common::string_or_regex> elements)
    : filter_visitor{type}
    , elements_{std::move(elements)}
{
}

tvl::value_t element_filter::match(const diagram &d, const element &e) const
{
    // Do not apply element filter to packages in class diagrams
    if (d.type() == diagram_t::kClass && e.type_name() == "package")
        return std::nullopt;

    return tvl::any_of(
        elements_.begin(), elements_.end(), [&e](const auto &el) {
            return ((el == e.full_name(false)) ||
                (el == fmt::format("::{}", e.full_name(false))));
        });
}

tvl::value_t element_filter::match(
    const diagram &d, const sequence_diagram::model::participant &p) const
{
    using sequence_diagram::model::method;
    using sequence_diagram::model::participant;

    if (d.type() != diagram_t::kSequence)
        return {};

    const auto &sequence_model =
        dynamic_cast<const sequence_diagram::model::diagram &>(d);
    return tvl::any_of(elements_.begin(), elements_.end(),
        [&sequence_model, &p](const auto &el) {
            if (p.type_name() == "method") {

                const auto &m = dynamic_cast<const method &>(p);
                const auto class_id = m.class_id();
                const auto &class_participant =
                    sequence_model.get_participant<participant>(class_id)
                        .value();

                return el == p.full_name(false) ||
                    el == class_participant.full_name(false);
            }

            return el == p.full_name(false);
        });
}

element_type_filter::element_type_filter(
    filter_t type, std::vector<std::string> element_types)
    : filter_visitor{type}
    , element_types_{std::move(element_types)}
{
}

tvl::value_t element_type_filter::match(
    const diagram & /*d*/, const element &e) const
{
    return tvl::any_of(element_types_.begin(), element_types_.end(),
        [&e](const auto &element_type) {
            return e.type_name() == element_type;
        });
}

method_type_filter::method_type_filter(
    filter_t type, std::vector<config::method_type> method_types)
    : filter_visitor{type}
    , method_types_{std::move(method_types)}
{
}

tvl::value_t method_type_filter::match(
    const diagram & /*d*/, const class_diagram::model::class_method &m) const
{
    return tvl::any_of(
        method_types_.begin(), method_types_.end(), [&m](auto mt) {
            switch (mt) {
            case config::method_type::constructor:
                return m.is_constructor();
            case config::method_type::destructor:
                return m.is_destructor();
            case config::method_type::assignment:
                return m.is_copy_assignment() || m.is_move_assignment();
            case config::method_type::operator_:
                return m.is_operator();
            case config::method_type::defaulted:
                return m.is_defaulted();
            case config::method_type::deleted:
                return m.is_deleted();
            case config::method_type::static_:
                return m.is_static();
            }

            return false;
        });
}

callee_filter::callee_filter(
    filter_t type, std::vector<config::callee_type> callee_types)
    : filter_visitor{type}
    , callee_types_{std::move(callee_types)}
{
}

tvl::value_t callee_filter::match(
    const diagram &d, const sequence_diagram::model::participant &p) const
{
    using sequence_diagram::model::class_;
    using sequence_diagram::model::function;
    using sequence_diagram::model::method;
    using sequence_diagram::model::participant;

    auto is_lambda = [&d](const method &m) {
        auto class_participant =
            dynamic_cast<const sequence_diagram::model::diagram &>(d)
                .get_participant<class_>(m.class_id());
        if (!class_participant)
            return false;

        return class_participant.value().is_lambda();
    };

    tvl::value_t res = tvl::any_of(
        callee_types_.begin(), callee_types_.end(), [&p, is_lambda](auto ct) {
            auto is_function = [](const participant *p) {
                return dynamic_cast<const function *>(p) != nullptr;
            };

            auto is_cuda_kernel = [](const participant *p) {
                const auto *f = dynamic_cast<const function *>(p);
                return (f != nullptr) && (f->is_cuda_kernel());
            };

            auto is_cuda_device = [](const participant *p) {
                const auto *f = dynamic_cast<const function *>(p);
                return (f != nullptr) && (f->is_cuda_device());
            };

            switch (ct) {
            case config::callee_type::method:
                return p.type_name() == "method";
            case config::callee_type::constructor:
                return p.type_name() == "method" &&
                    ((method &)p).is_constructor();
            case config::callee_type::assignment:
                return p.type_name() == "method" &&
                    ((method &)p).is_assignment();
            case config::callee_type::operator_:
                return is_function(&p) && ((function &)p).is_operator();
            case config::callee_type::defaulted:
                return p.type_name() == "method" &&
                    ((method &)p).is_defaulted();
            case config::callee_type::static_:
                return is_function(&p) && ((function &)p).is_static();
            case config::callee_type::function:
                return p.type_name() == "function";
            case config::callee_type::function_template:
                return p.type_name() == "function_template";
            case config::callee_type::lambda:
                return p.type_name() == "method" && is_lambda((method &)p);
            case config::callee_type::cuda_kernel:
                return is_cuda_kernel(&p);
            case config::callee_type::cuda_device:
                return is_cuda_device(&p);
            }

            return false;
        });

    return res;
}

subclass_filter::subclass_filter(
    filter_t type, std::vector<common::string_or_regex> roots)
    : filter_visitor{type}
    , roots_{std::move(roots)}
{
}

tvl::value_t subclass_filter::match(const diagram &d, const element &e) const
{
    if (d.type() != diagram_t::kClass)
        return {};

    if (roots_.empty())
        return {};

    if (!d.complete())
        return {};

    const auto &cd = dynamic_cast<const class_diagram::model::diagram &>(d);

    // First get all parents of element e
    clanguml::common::reference_set<class_diagram::model::class_> parents;

    const auto &fn = e.full_name(false);
    auto class_ref = cd.find<class_diagram::model::class_>(fn);

    if (!class_ref.has_value())
        return false;

    parents.emplace(class_ref.value());

    cd.get_parents(parents);

    std::vector<std::string> parents_names;
    for (const auto p : parents)
        parents_names.push_back(p.get().full_name(false));

    // Now check if any of the parents matches the roots specified in the
    // filter config
    for (const auto &root : roots_) {
        for (const auto &parent : parents) {
            auto full_name = parent.get().full_name(false);
            if (root == full_name) {
                return true;
            }
        }
    }

    return false;
}

parents_filter::parents_filter(
    filter_t type, std::vector<common::string_or_regex> children)
    : filter_visitor{type}
    , children_{std::move(children)}
{
}

tvl::value_t parents_filter::match(const diagram &d, const element &e) const
{
    if (d.type() != diagram_t::kClass)
        return {};

    if (children_.empty())
        return {};

    if (!d.complete())
        return {};

    const auto &cd = dynamic_cast<const class_diagram::model::diagram &>(d);

    // First get all parents of element e
    clanguml::common::reference_set<class_diagram::model::class_> parents;

    for (const auto &child_pattern : children_) {
        auto child_refs = cd.find<class_diagram::model::class_>(child_pattern);

        for (auto &child : child_refs) {
            if (child.has_value())
                parents.emplace(child.value());
        }
    }

    cd.get_parents(parents);

    for (const auto &parent : parents) {
        if (e == parent.get())
            return true;
    }

    return false;
}

relationship_filter::relationship_filter(
    filter_t type, std::vector<relationship_t> relationships)
    : filter_visitor{type}
    , relationships_{std::move(relationships)}
{
}

tvl::value_t relationship_filter::match(
    const diagram & /*d*/, const relationship_t &r) const
{
    return tvl::any_of(relationships_.begin(), relationships_.end(),
        [&r](const auto &rel) { return r == rel; });
}

access_filter::access_filter(filter_t type, std::vector<access_t> access)
    : filter_visitor{type}
    , access_{std::move(access)}
{
}

tvl::value_t access_filter::match(
    const diagram & /*d*/, const access_t &a) const
{
    return tvl::any_of(access_.begin(), access_.end(),
        [&a](const auto &access) { return a == access; });
}

module_access_filter::module_access_filter(
    filter_t type, std::vector<module_access_t> access)
    : filter_visitor{type}
    , access_{std::move(access)}
{
}

tvl::value_t module_access_filter::match(
    const diagram & /*d*/, const element &e) const
{
    if (!e.module().has_value())
        return {};

    if (access_.empty())
        return {};

    return tvl::any_of(
        access_.begin(), access_.end(), [&e](const auto &access) {
            if (access == module_access_t::kPublic)
                return !e.module_private();

            return e.module_private();
        });
}

context_filter::context_filter(
    filter_t type, std::vector<config::context_config> context)
    : filter_visitor{type}
    , context_{std::move(context)}
{
}

void context_filter::initialize_effective_context(
    const diagram &d, unsigned idx) const
{
    bool effective_context_extended{true};

    auto &effective_context = effective_contexts_[idx];

    // First add to effective context all elements matching context_ patterns
    const auto &context_cfg = context_.at(idx);
    const auto &context_matches =
        dynamic_cast<const class_diagram::model::diagram &>(d)
            .find<class_diagram::model::class_>(context_cfg.pattern);

    for (const auto &maybe_match : context_matches) {
        if (maybe_match)
            effective_context.emplace(maybe_match.value().id());
    }

    const auto &context_enum_matches =
        dynamic_cast<const class_diagram::model::diagram &>(d)
            .find<class_diagram::model::enum_>(context_cfg.pattern);

    for (const auto &maybe_match : context_enum_matches) {
        if (maybe_match)
            effective_context.emplace(maybe_match.value().id());
    }

    const auto &context_concept_matches =
        dynamic_cast<const class_diagram::model::diagram &>(d)
            .find<class_diagram::model::concept_>(context_cfg.pattern);

    for (const auto &maybe_match : context_concept_matches) {
        if (maybe_match)
            effective_context.emplace(maybe_match.value().id());
    }

    // Now repeat radius times - extend the effective context with elements
    // matching in direct relationship to what is in context
    auto radius_counter = context_cfg.radius;
    std::set<eid_t> current_iteration_context;

    while (radius_counter > 0 && effective_context_extended) {
        // If at any iteration the effective context was not extended - we
        // don't to need to continue
        radius_counter--;
        effective_context_extended = false;
        current_iteration_context.clear();

        // For each class in the model
        find_elements_in_direct_relationship<class_diagram::model::class_>(
            d, context_cfg, effective_context, current_iteration_context);

        // For each concept in the model
        find_elements_in_direct_relationship<class_diagram::model::concept_>(
            d, context_cfg, effective_context, current_iteration_context);

        // For each enum in the model
        find_elements_in_direct_relationship<class_diagram::model::enum_>(
            d, context_cfg, effective_context, current_iteration_context);

        for (auto id : current_iteration_context) {
            if (effective_context.count(id) == 0) {
                // Found new element to add to context
                effective_context.emplace(id);
                effective_context_extended = true;
            }
        }
    }
}

bool context_filter::should_include(
    const config::context_config &context_cfg, relationship_t r) const
{
    return context_cfg.relationships.empty() ||
        util::contains(context_cfg.relationships, r);
}

void context_filter::initialize(const diagram &d) const
{
    if (initialized_)
        return;

    initialized_ = true;

    // Prepare effective_contexts_
    for (auto i = 0U; i < context_.size(); i++) {
        effective_contexts_.push_back({}); // NOLINT
        initialize_effective_context(d, i);
    }
}

tvl::value_t context_filter::match(const diagram &d, const element &e) const
{
    if (d.type() != diagram_t::kClass)
        return {};

    // Running this filter makes sense only after the entire diagram is
    // generated - i.e. during diagram generation
    if (!d.complete())
        return {};

    initialize(d);

    if (std::all_of(effective_contexts_.begin(), effective_contexts_.end(),
            [](const auto &ec) { return ec.empty(); }))
        return {};

    for (const auto &ec : effective_contexts_) {
        if (ec.count(e.id()) > 0)
            return true;
    }

    return false;
}

bool context_filter::is_inward(relationship_t r) const
{
    return r == relationship_t::kAssociation;
}

bool context_filter::is_outward(relationship_t r) const
{
    return r != relationship_t::kAssociation;
}

paths_filter::paths_filter(filter_t type, const std::vector<std::string> &p,
    const std::filesystem::path &root)
    : filter_visitor{type}
    , root_{root}
{
    for (const auto &path : p) {
        std::filesystem::path absolute_path;

        if (path.empty() || path == ".")
            absolute_path = root;
        else if (std::filesystem::path{path}.is_relative())
            absolute_path = root / path;
        else
            absolute_path = path;

        bool match_successful{false};
        for (auto &resolved_glob_path :
            glob::glob(absolute_path.string(), true)) {
            try {
                auto resolved_absolute_path = absolute(resolved_glob_path);
                resolved_absolute_path =
                    canonical(resolved_absolute_path.lexically_normal());

                resolved_absolute_path.make_preferred();

                LOG_DBG("Added path {} to paths_filter",
                    resolved_absolute_path.string());

                paths_.emplace_back(std::move(resolved_absolute_path));

                match_successful = true;
            }
            catch (std::filesystem::filesystem_error &e) {
                LOG_WARN("Cannot add non-existent path {} to "
                         "paths filter",
                    absolute_path.string());
                continue;
            }
        }

        if (!match_successful)
            LOG_WARN("Paths filter pattern '{}' did not match "
                     "any files relative to '{}'",
                path, root_.string());
    }
}

tvl::value_t paths_filter::match(
    const diagram & /*d*/, const common::model::source_file &p) const
{
    if (paths_.empty()) {
        return {};
    }

    // Matching source paths doesn't make sense if they are not absolute
    if (!p.is_absolute()) {
        return {};
    }

    auto pp = p.fs_path(root_);
    for (const auto &path : paths_) {
        if (pp.root_name().string() == path.root_name().string() &&
            util::starts_with(pp.relative_path(), path.relative_path())) {

            return true;
        }
    }

    return false;
}

tvl::value_t paths_filter::match(
    const diagram & /*d*/, const common::model::source_location &p) const
{
    if (paths_.empty()) {
        return {};
    }

    auto sl_path = std::filesystem::path{p.file()};

    // Matching source paths doesn't make sens if they are not absolute or
    // empty
    if (p.file().empty() || sl_path.is_relative()) {
        return {};
    }

    for (const auto &path : paths_) {
        if (sl_path.root_name().string() == path.root_name().string() &&
            util::starts_with(sl_path.relative_path(), path.relative_path())) {

            return true;
        }
    }

    return false;
}

class_method_filter::class_method_filter(filter_t type,
    std::unique_ptr<access_filter> af, std::unique_ptr<method_type_filter> mtf)
    : filter_visitor{type}
    , access_filter_{std::move(af)}
    , method_type_filter_{std::move(mtf)}
{
}

tvl::value_t class_method_filter::match(
    const diagram &d, const class_diagram::model::class_method &m) const
{
    tvl::value_t res = tvl::or_(
        access_filter_->match(d, m.access()), method_type_filter_->match(d, m));

    return res;
}

class_member_filter::class_member_filter(
    filter_t type, std::unique_ptr<access_filter> af)
    : filter_visitor{type}
    , access_filter_{std::move(af)}
{
}

tvl::value_t class_member_filter::match(
    const diagram &d, const class_diagram::model::class_member &m) const
{
    return access_filter_->match(d, m.access());
}

diagram_filter::diagram_filter(const common::model::diagram &d,
    const config::diagram & /*c*/, private_constructor_tag_t /*unused*/)
    : diagram_{d}
{
}

void diagram_filter::add_filter(
    filter_t filter_type, std::unique_ptr<filter_visitor> fv)
{
    if (filter_type == filter_t::kInclusive)
        add_inclusive_filter(std::move(fv));
    else
        add_exclusive_filter(std::move(fv));
}

void diagram_filter::add_inclusive_filter(std::unique_ptr<filter_visitor> fv)
{
    inclusive_.emplace_back(std::move(fv));
}

void diagram_filter::add_exclusive_filter(std::unique_ptr<filter_visitor> fv)
{
    exclusive_.emplace_back(std::move(fv));
}

bool diagram_filter::should_include(
    const namespace_ &ns, const std::string &name) const
{
    if (should_include(ns)) {
        element e{namespace_{}};
        e.set_name(name);
        e.set_namespace(ns);

        return should_include(e);
    }

    return false;
}

template <>
bool diagram_filter::should_include<std::string>(const std::string &name) const
{
    if (name.empty())
        return false;

    auto [ns, n] = common::split_ns(name);

    return should_include(ns, n);
}
} // namespace clanguml::common::model
