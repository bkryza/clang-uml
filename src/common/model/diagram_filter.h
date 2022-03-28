/**
 * src/common/model/entity_filter.h
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
#pragma once

#include "class_diagram/model/diagram.h"
#include "common/model/diagram.h"
#include "common/model/element.h"
#include "common/model/enums.h"
#include "common/model/namespace.h"
#include "config/config.h"
#include "cx/util.h"
#include "diagram.h"

namespace clanguml::common::model {

enum filter_t { kInclusive, kExclusive };

class filter_visitor {
public:
    filter_visitor(filter_t type)
        : type_{type}
    {
    }

    virtual std::optional<bool> match(
        const diagram &d, const common::model::element &e);
    virtual std::optional<bool> match(
        const diagram &d, const common::model::relationship_t &r);
    virtual std::optional<bool> match(
        const diagram &d, const common::model::scope_t &r);
    virtual std::optional<bool> match(
        const diagram &d, const common::model::namespace_ &ns);

    bool is_inclusive() const { return type_ == filter_t::kInclusive; }
    bool is_exclusive() const { return type_ == filter_t::kExclusive; }

    filter_t type_;
};

struct namespace_filter : public filter_visitor {
    namespace_filter(filter_t type, std::vector<namespace_> namespaces)
        : filter_visitor{type}
        , namespaces_{namespaces}
    {
    }

    std::optional<bool> match(const diagram &d, const namespace_ &ns) override
    {
        if (namespaces_.empty() || ns.is_empty())
            return {};

        return std::any_of(
            namespaces_.begin(), namespaces_.end(), [&ns](const auto &nsit) {
                return ns.starts_with(nsit) || ns == nsit;
            });
    }

    std::optional<bool> match(const diagram &d, const element &e) override
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

    std::vector<namespace_> namespaces_;
};

struct element_filter : public filter_visitor {
    element_filter(filter_t type, std::vector<std::string> elements)
        : filter_visitor{type}
        , elements_{elements}
    {
    }

    std::optional<bool> match(const diagram &d, const element &e) override
    {
        if (elements_.empty())
            return {};

        return std::any_of(
            elements_.begin(), elements_.end(), [&e](const auto &el) {
                auto fn = e.full_name(false);
                bool result = fn == el;
                return result;
            });
    }

    std::vector<std::string> elements_;
};

struct subclass_filter : public filter_visitor {
    subclass_filter(filter_t type, std::vector<std::string> roots)
        : filter_visitor{type}
        , roots_{roots}
    {
    }

    std::optional<bool> match(const diagram &d, const element &e) override
    {
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

    std::vector<std::string> roots_;
};

struct relationship_filter : public filter_visitor {
    relationship_filter(filter_t type, std::vector<std::string> relationships)
        : filter_visitor{type}
        , relationships_{relationships}
    {
    }

    std::optional<bool> match(
        const diagram &d, const relationship_t &r) override
    {
        if (relationships_.empty())
            return {};

        return std::any_of(relationships_.begin(), relationships_.end(),
            [&r](const auto &rel) {
                bool res = to_string(r) == rel;
                return res;
            });
    }

    std::vector<std::string> relationships_;
};

struct scope_filter : public filter_visitor {
    scope_filter(filter_t type, std::vector<std::string> scopes)
        : filter_visitor{type}
        , scopes_{scopes}
    {
    }

    std::optional<bool> match(const diagram &d, const scope_t &s) override
    {
        if (scopes_.empty())
            return {};

        return std::any_of(
            scopes_.begin(), scopes_.end(), [&s](const auto &rel) {
                bool res = to_string(s) == rel;
                return res;
            });
    }

    std::vector<std::string> scopes_;
};

struct context_filter : public filter_visitor {
    context_filter(filter_t type, std::vector<std::string> context)
        : filter_visitor{type}
        , context_{context}
    {
    }

    std::optional<bool> match(const diagram &d, const element &r) override
    {
        if (!d.complete())
            return {};

        if (context_.empty())
            return {};

        return std::any_of(context_.begin(), context_.end(),
            [&r](const auto &rel) { return std::optional<bool>{}; });
    }

    std::vector<std::string> context_;
};

class diagram_filter {
public:
    diagram_filter(const common::model::diagram &d, const config::diagram &c)
        : diagram_{d}
    {
        init_filters(c);
    }

    void add_inclusive_filter(std::unique_ptr<filter_visitor> fv)
    {
        inclusive_.emplace_back(std::move(fv));
    }

    void add_exclusive_filter(std::unique_ptr<filter_visitor> fv)
    {
        exclusive_.emplace_back(std::move(fv));
    }

    bool should_include(namespace_ ns, const std::string &name)
    {
        if (should_include(ns)) {
            element e{namespace_{}};
            e.set_name(name);
            e.set_namespace(ns);

            return should_include(e);
        }

        return false;
    }

    template <typename T> bool should_include(const T &e)
    {
        bool exc = std::any_of(
            exclusive_.begin(), exclusive_.end(), [this, &e](const auto &ex) {
                auto m = ex->match(diagram_, e);
                // Return a match if a filter is undefined for specific element
                // or it's a match
                return m.has_value() && m.value();
            });
        if (exc)
            return false;

        bool inc = std::all_of(
            inclusive_.begin(), inclusive_.end(), [this, &e](const auto &in) {
                auto m = in->match(diagram_, e);
                // Return a match if a filter is undefined for specific element
                // or it's a match
                return !m.has_value() || m.value();
            });

        if (inc)
            return true;

        return false;
    }

private:
    void init_filters(const config::diagram &c)
    {
        // Process inclusive filters
        if (c.include) {
            inclusive_.emplace_back(std::make_unique<namespace_filter>(
                filter_t::kInclusive, c.include().namespaces));
            inclusive_.emplace_back(std::make_unique<relationship_filter>(
                filter_t::kInclusive, c.include().relationships));
            inclusive_.emplace_back(std::make_unique<scope_filter>(
                filter_t::kInclusive, c.include().scopes));
            inclusive_.emplace_back(std::make_unique<element_filter>(
                filter_t::kInclusive, c.include().elements));
            inclusive_.emplace_back(std::make_unique<subclass_filter>(
                filter_t::kInclusive, c.include().subclasses));
            inclusive_.emplace_back(std::make_unique<context_filter>(
                filter_t::kInclusive, c.include().context));
        }

        // Process exclusive filters
        if (c.exclude) {
            exclusive_.emplace_back(std::make_unique<namespace_filter>(
                filter_t::kExclusive, c.exclude().namespaces));
            exclusive_.emplace_back(std::make_unique<element_filter>(
                filter_t::kExclusive, c.exclude().elements));
            exclusive_.emplace_back(std::make_unique<relationship_filter>(
                filter_t::kExclusive, c.include().relationships));
            exclusive_.emplace_back(std::make_unique<scope_filter>(
                filter_t::kExclusive, c.include().scopes));
            exclusive_.emplace_back(std::make_unique<subclass_filter>(
                filter_t::kExclusive, c.exclude().subclasses));
            exclusive_.emplace_back(std::make_unique<context_filter>(
                filter_t::kExclusive, c.exclude().context));
        }
    }

    std::vector<std::unique_ptr<filter_visitor>> inclusive_;
    std::vector<std::unique_ptr<filter_visitor>> exclusive_;

    const common::model::diagram &diagram_;
};

template <>
bool diagram_filter::should_include<std::string>(const std::string &name);
}