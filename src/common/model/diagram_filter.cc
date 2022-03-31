/**
 * src/common/model/diagram_filter.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

namespace clanguml::common::model {

filter_visitor::filter_visitor(filter_t type)
    : type_{type}
{
}

std::optional<bool> filter_visitor::match(
    const diagram &d, const common::model::element &e) const
{
    return {};
}

std::optional<bool> filter_visitor::match(
    const diagram &d, const common::model::relationship_t &r) const
{
    return {};
}

std::optional<bool> filter_visitor::match(
    const diagram &d, const common::model::access_t &a) const
{
    return {};
}

std::optional<bool> filter_visitor::match(
    const diagram &d, const common::model::namespace_ &ns) const
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

anyof_filter::anyof_filter(
    filter_t type, std::vector<std::unique_ptr<filter_visitor>> filters)
    : filter_visitor{type}
    , filters_{std::move(filters)}
{
}

std::optional<bool> anyof_filter::match(
    const diagram &d, const common::model::element &e) const
{
    std::optional<bool> res{};

    for (const auto &filter : filters_) {
        auto m = filter->match(d, e);
        if (m.has_value()) {
            if (m.value() == true) {
                res = true;
                break;
            }
            else {
                res = false;
            }
        }
    }

    return res;
}

namespace_filter::namespace_filter(
    filter_t type, std::vector<namespace_> namespaces)
    : filter_visitor{type}
    , namespaces_{namespaces}
{
}

std::optional<bool> namespace_filter::match(
    const diagram &d, const namespace_ &ns) const
{
    if (namespaces_.empty() || ns.is_empty())
        return {};

    return std::any_of(namespaces_.begin(), namespaces_.end(),
        [&ns](const auto &nsit) { return ns.starts_with(nsit) || ns == nsit; });
}

std::optional<bool> namespace_filter::match(
    const diagram &d, const element &e) const
{
    if (namespaces_.empty())
        return {};

    if (dynamic_cast<const package *>(&e) != nullptr) {
        return std::any_of(
            namespaces_.begin(), namespaces_.end(), [&e](const auto &nsit) {
                return (e.get_namespace() | e.name()).starts_with(nsit) ||
                    nsit.starts_with(e.get_namespace() | e.name()) ||
                    (e.get_namespace() | e.name()) == nsit;
            });
    }
    else {
        return std::any_of(
            namespaces_.begin(), namespaces_.end(), [&e](const auto &nsit) {
                return e.get_namespace().starts_with(nsit);
            });
    }
}

element_filter::element_filter(filter_t type, std::vector<std::string> elements)
    : filter_visitor{type}
    , elements_{elements}
{
}

std::optional<bool> element_filter::match(
    const diagram &d, const element &e) const
{
    if (elements_.empty())
        return {};

    return std::any_of(elements_.begin(), elements_.end(),
        [&e](const auto &el) { return e.full_name(false) == el; });
}

subclass_filter::subclass_filter(filter_t type, std::vector<std::string> roots)
    : filter_visitor{type}
    , roots_{roots}
{
}

std::optional<bool> subclass_filter::match(
    const diagram &d, const element &e) const
{
    if (d.type() != diagram_t::kClass)
        return {};

    if (roots_.empty())
        return {};

    if (!d.complete())
        return {};

    const auto &cd = dynamic_cast<const class_diagram::model::diagram &>(d);

    // First get all parents of element e
    std::unordered_set<
        type_safe::object_ref<const class_diagram::model::class_, false>>
        parents;

    const auto &fn = e.full_name(false);
    auto class_ref = cd.get_class(fn);

    if (!class_ref.has_value())
        return false;

    parents.emplace(class_ref.value());

    cd.get_parents(parents);

    // Now check if any of the parents matches the roots specified in the
    // filter config
    for (const auto &root : roots_) {
        for (const auto &parent : parents) {
            if (root == parent.get().full_name(false))
                return true;
        }
    }

    return false;
}

relationship_filter::relationship_filter(
    filter_t type, std::vector<relationship_t> relationships)
    : filter_visitor{type}
    , relationships_{relationships}
{
}

std::optional<bool> relationship_filter::match(
    const diagram &d, const relationship_t &r) const
{
    if (relationships_.empty())
        return {};

    return std::any_of(relationships_.begin(), relationships_.end(),
        [&r](const auto &rel) { return r == rel; });
}

access_filter::access_filter(filter_t type, std::vector<access_t> access)
    : filter_visitor{type}
    , access_{access}
{
}

std::optional<bool> access_filter::match(
    const diagram &d, const access_t &a) const
{
    if (access_.empty())
        return {};

    return std::any_of(access_.begin(), access_.end(),
        [&a](const auto &access) { return a == access; });
}

context_filter::context_filter(filter_t type, std::vector<std::string> context)
    : filter_visitor{type}
    , context_{context}
{
}

std::optional<bool> context_filter::match(
    const diagram &d, const element &e) const
{
    if (d.type() != diagram_t::kClass)
        return {};

    if (!d.complete())
        return {};

    if (context_.empty())
        return {};

    return std::any_of(context_.begin(), context_.end(),
        [&e, &d](const auto &context_root_name) {
            const auto &context_root =
                static_cast<const class_diagram::model::diagram &>(d).get_class(
                    context_root_name);

            if (context_root.has_value()) {
                // This is a direct match to the context root
                if (context_root.value().full_name(false) == e.full_name(false))
                    return true;

                // Return a positive match if the element e is in a direct
                // relationship with any of the context_root's
                for (const relationship &rel :
                    context_root.value().relationships()) {
                    if (rel.destination() == e.full_name(false))
                        return true;
                }
                for (const relationship &rel : e.relationships()) {
                    if (rel.destination() ==
                        context_root.value().full_name(false))
                        return true;
                }

                // Return a positive match if the context_root is a parent
                // of the element
                for (const class_diagram::model::class_parent &p :
                    context_root.value().parents()) {
                    if (p.name() == e.full_name(false))
                        return true;
                }
                if (dynamic_cast<const class_diagram::model::class_ *>(&e) !=
                    nullptr) {
                    for (const class_diagram::model::class_parent &p :
                        static_cast<const class_diagram::model::class_ &>(e)
                            .parents()) {
                        if (p.name() == context_root.value().full_name(false))
                            return true;
                    }
                }
            }

            return false;
        });
}

diagram_filter::diagram_filter(
    const common::model::diagram &d, const config::diagram &c)
    : diagram_{d}
{
    init_filters(c);
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
    namespace_ ns, const std::string &name) const
{
    if (should_include(ns)) {
        element e{namespace_{}};
        e.set_name(name);
        e.set_namespace(ns);

        return should_include(e);
    }

    return false;
}

void diagram_filter::init_filters(const config::diagram &c)
{
    // Process inclusive filters
    if (c.include) {
        inclusive_.emplace_back(std::make_unique<namespace_filter>(
            filter_t::kInclusive, c.include().namespaces));
        inclusive_.emplace_back(std::make_unique<relationship_filter>(
            filter_t::kInclusive, c.include().relationships));
        inclusive_.emplace_back(std::make_unique<access_filter>(
            filter_t::kInclusive, c.include().access));

        // Include any of these matches even if one them does not match
        std::vector<std::unique_ptr<filter_visitor>> element_filters;
        element_filters.emplace_back(std::make_unique<element_filter>(
            filter_t::kInclusive, c.include().elements));
        element_filters.emplace_back(std::make_unique<subclass_filter>(
            filter_t::kInclusive, c.include().subclasses));
        element_filters.emplace_back(std::make_unique<context_filter>(
            filter_t::kInclusive, c.include().context));

        inclusive_.emplace_back(std::make_unique<anyof_filter>(
            filter_t::kInclusive, std::move(element_filters)));
    }

    // Process exclusive filters
    if (c.exclude) {
        exclusive_.emplace_back(std::make_unique<namespace_filter>(
            filter_t::kExclusive, c.exclude().namespaces));
        exclusive_.emplace_back(std::make_unique<element_filter>(
            filter_t::kExclusive, c.exclude().elements));
        exclusive_.emplace_back(std::make_unique<relationship_filter>(
            filter_t::kExclusive, c.exclude().relationships));
        exclusive_.emplace_back(std::make_unique<access_filter>(
            filter_t::kExclusive, c.exclude().access));
        exclusive_.emplace_back(std::make_unique<subclass_filter>(
            filter_t::kExclusive, c.exclude().subclasses));
        exclusive_.emplace_back(std::make_unique<context_filter>(
            filter_t::kExclusive, c.exclude().context));
    }
}

template <>
bool diagram_filter::should_include<std::string>(const std::string &name) const
{
    auto [ns, n] = cx::util::split_ns(name);

    return should_include(ns, n);
}

}
