/**
 * src/common/model/diagram_filter.h
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
#include "source_file.h"
#include "tvl.h"

#include <filesystem>

namespace clanguml::common::model {

enum filter_t { kInclusive, kExclusive };

namespace detail {
template <typename ElementT, typename DiagramT>
const std::vector<type_safe::object_ref<const ElementT>> &view(
    const DiagramT &d);

template <typename ElementT, typename DiagramT>
const type_safe::optional_ref<const ElementT> get(
    const DiagramT &d, const std::string &full_name);
} // namespace detail

class filter_visitor {
public:
    filter_visitor(filter_t type);

    virtual tvl::value_t match(
        const diagram &d, const common::model::element &e) const;

    virtual tvl::value_t match(
        const diagram &d, const common::model::relationship_t &r) const;

    virtual tvl::value_t match(
        const diagram &d, const common::model::access_t &a) const;

    virtual tvl::value_t match(
        const diagram &d, const common::model::namespace_ &ns) const;

    virtual tvl::value_t match(
        const diagram &d, const common::model::source_file &f) const;

    bool is_inclusive() const;
    bool is_exclusive() const;

    filter_t type() const;

private:
    filter_t type_;
};

struct anyof_filter : public filter_visitor {
    anyof_filter(
        filter_t type, std::vector<std::unique_ptr<filter_visitor>> filters);

    tvl::value_t match(
        const diagram &d, const common::model::element &e) const override;

private:
    std::vector<std::unique_ptr<filter_visitor>> filters_;
};

struct namespace_filter : public filter_visitor {
    namespace_filter(filter_t type, std::vector<namespace_> namespaces);

    tvl::value_t match(const diagram &d, const namespace_ &ns) const override;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<namespace_> namespaces_;
};

struct element_filter : public filter_visitor {
    element_filter(filter_t type, std::vector<std::string> elements);

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<std::string> elements_;
};

struct subclass_filter : public filter_visitor {
    subclass_filter(filter_t type, std::vector<std::string> roots);

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<std::string> roots_;
};

template <typename DiagramT, typename ElementT>
struct tree_element_filter : public filter_visitor {
    tree_element_filter(filter_t type, relationship_t relationship,
        std::vector<std::string> roots, bool forward = false)
        : filter_visitor{type}
        , relationship_{relationship}
        , roots_{roots}
        , forward_{forward}
    {
    }

    tvl::value_t match(const diagram &d, const element &e) const override
    {
        if (roots_.empty())
            return {};

        // This filter should only be run on the completely generated diagram
        // model by visitor
        if (!d.complete())
            return {};

        const auto &cd = dynamic_cast<const DiagramT &>(d);

        // Calculate the set of matching elements
        init(cd);

        const auto &fn = e.full_name(false);
        auto element_ref = detail::get<ElementT>(cd, fn);

        if (!element_ref.has_value())
            // Couldn't find the element in the diagram
            return false;

        // Now check if the e element is contained in the calculated set
        const auto &e_full_name = e.full_name(false);
        bool res =
            std::find_if(matching_elements_.begin(), matching_elements_.end(),
                [&e_full_name](const auto &te) {
                    return te->full_name(false) == e_full_name;
                }) != matching_elements_.end();

        return res;
    }

private:
    void init(const DiagramT &cd) const
    {
        if (initialized_)
            return;

        // First get all elements specified in the filter configuration
        for (const auto &template_root : roots_) {
            auto template_ref = detail::get<ElementT>(cd, template_root);
            if (template_ref.has_value())
                matching_elements_.emplace(template_ref.value());
        }

        auto match_tree_rel = [&, this](const auto &from, const auto &to) {
            bool added_new_element{false};

            for (const auto &from_el : from) {
                //  Check if any of its relationships of type relationship_
                //  points to an element already in the matching_elements_
                //  set
                for (const auto &rel : from_el->relationships()) {
                    if (rel.type() == relationship_) {
                        for (const auto &to_el : to) {
                            if (rel.destination() == to_el->full_name(false)) {
                                const auto &to_add = forward_ ? to_el : from_el;
                                if (matching_elements_.insert(to_add).second)
                                    added_new_element = true;
                            }
                        }
                    }
                }
            }

            return added_new_element;
        };

        bool keep_looking{true};
        while (keep_looking) {
            keep_looking = false;
            if (forward_) {
                if (match_tree_rel(
                        matching_elements_, detail::view<ElementT>(cd)))
                    keep_looking = true;
            }
            else {
                if (match_tree_rel(
                        detail::view<ElementT>(cd), matching_elements_))
                    keep_looking = true;
            }
        }

        initialized_ = true;
    }

    std::vector<std::string> roots_;
    relationship_t relationship_;
    mutable bool initialized_{false};
    mutable std::unordered_set<type_safe::object_ref<const ElementT, false>>
        matching_elements_;
    bool forward_;
};

struct relationship_filter : public filter_visitor {
    relationship_filter(
        filter_t type, std::vector<relationship_t> relationships);

    tvl::value_t match(
        const diagram &d, const relationship_t &r) const override;

private:
    std::vector<relationship_t> relationships_;
};

struct access_filter : public filter_visitor {
    access_filter(filter_t type, std::vector<access_t> access);

    tvl::value_t match(const diagram &d, const access_t &a) const override;

private:
    std::vector<access_t> access_;
};

struct context_filter : public filter_visitor {
    context_filter(filter_t type, std::vector<std::string> context);

    tvl::value_t match(const diagram &d, const element &r) const override;

private:
    std::vector<std::string> context_;
};

struct paths_filter : public filter_visitor {
    paths_filter(filter_t type, const std::filesystem::path &root,
        std::vector<std::filesystem::path> p);

    tvl::value_t match(
        const diagram &d, const common::model::source_file &r) const override;

private:
    std::vector<std::filesystem::path> paths_;
    std::filesystem::path root_;
};

class diagram_filter {
public:
    diagram_filter(const common::model::diagram &d, const config::diagram &c);

    void add_inclusive_filter(std::unique_ptr<filter_visitor> fv);

    void add_exclusive_filter(std::unique_ptr<filter_visitor> fv);

    bool should_include(namespace_ ns, const std::string &name) const;

    template <typename T> bool should_include(const T &e) const
    {
        auto exc = tvl::any_of(exclusive_.begin(), exclusive_.end(),
            [this, &e](const auto &ex) { return ex->match(diagram_, e); });

        if (tvl::is_true(exc))
            return false;

        auto inc = tvl::all_of(inclusive_.begin(), inclusive_.end(),
            [this, &e](const auto &in) { return in->match(diagram_, e); });

        if (tvl::is_undefined(inc) || tvl::is_true(inc))
            return true;

        return false;
    }

private:
    void init_filters(const config::diagram &c);

    std::vector<std::unique_ptr<filter_visitor>> inclusive_;
    std::vector<std::unique_ptr<filter_visitor>> exclusive_;

    const common::model::diagram &diagram_;
};

template <>
bool diagram_filter::should_include<std::string>(const std::string &name) const;
}