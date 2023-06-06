/**
 * src/common/model/diagram_filter.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

#include "class_diagram/model/class_member.h"
#include "class_diagram/model/class_method.h"
#include "class_diagram/model/diagram.h"
#include "common/clang_utils.h"
#include "common/model/diagram.h"
#include "common/model/element.h"
#include "common/model/enums.h"
#include "common/model/namespace.h"
#include "config/config.h"
#include "diagram.h"
#include "include_diagram/model/diagram.h"
#include "source_file.h"
#include "tvl.h"

#include <filesystem>
#include <utility>

namespace clanguml::common::model {

enum filter_t { kInclusive, kExclusive };

namespace detail {
template <typename ElementT, typename DiagramT>
const clanguml::common::reference_vector<ElementT> &view(const DiagramT &d);

template <typename ElementT, typename DiagramT>
const clanguml::common::optional_ref<ElementT> get(
    const DiagramT &d, const std::string &full_name);

template <typename ElementT> int64_t destination_comparator(const ElementT &e)
{
    return e.id();
}

template <>
clanguml::common::id_t destination_comparator(
    const common::model::source_file &f);
} // namespace detail

class filter_visitor {
public:
    filter_visitor(filter_t type);

    virtual ~filter_visitor() = default;

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

    virtual tvl::value_t match(
        const diagram &d, const common::model::source_location &f) const;

    virtual tvl::value_t match(
        const diagram &d, const class_diagram::model::class_method &m) const;

    virtual tvl::value_t match(
        const diagram &d, const class_diagram::model::class_member &m) const;

    bool is_inclusive() const;
    bool is_exclusive() const;

    filter_t type() const;

private:
    filter_t type_;
};

struct anyof_filter : public filter_visitor {
    anyof_filter(
        filter_t type, std::vector<std::unique_ptr<filter_visitor>> filters);

    ~anyof_filter() override = default;

    tvl::value_t match(
        const diagram &d, const common::model::element &e) const override;

    tvl::value_t match(
        const diagram &d, const common::model::source_file &e) const override;

private:
    std::vector<std::unique_ptr<filter_visitor>> filters_;
};

struct namespace_filter : public filter_visitor {
    namespace_filter(
        filter_t type, std::vector<config::namespace_or_regex> namespaces);

    ~namespace_filter() override = default;

    tvl::value_t match(const diagram &d, const namespace_ &ns) const override;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<config::namespace_or_regex> namespaces_;
};

struct element_filter : public filter_visitor {
    element_filter(
        filter_t type, std::vector<config::string_or_regex> elements);

    ~element_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<config::string_or_regex> elements_;
};

struct element_type_filter : public filter_visitor {
    element_type_filter(filter_t type, std::vector<std::string> element_types);

    ~element_type_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<std::string> element_types_;
};

struct method_type_filter : public filter_visitor {
    method_type_filter(
        filter_t type, std::vector<config::method_type> method_types);

    ~method_type_filter() override = default;

    tvl::value_t match(const diagram &d,
        const class_diagram::model::class_method &e) const override;

private:
    std::vector<config::method_type> method_types_;
};

struct subclass_filter : public filter_visitor {
    subclass_filter(filter_t type, std::vector<config::string_or_regex> roots);

    ~subclass_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<config::string_or_regex> roots_;
};

struct parents_filter : public filter_visitor {
    parents_filter(filter_t type, std::vector<std::string> roots);

    ~parents_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<std::string> children_;
};

template <typename DiagramT, typename ElementT,
    typename MatchOverrideT = common::model::element>
struct edge_traversal_filter : public filter_visitor {
    edge_traversal_filter(filter_t type, relationship_t relationship,
        std::vector<std::string> roots, bool forward = false)
        : filter_visitor{type}
        , roots_{std::move(roots)}
        , relationship_{relationship}
        , forward_{forward}
    {
    }

    ~edge_traversal_filter() override = default;

    tvl::value_t match(const diagram &d, const MatchOverrideT &e) const override
    {
        // This filter should only be run on the completely generated diagram
        // model by visitor
        if (!d.complete())
            return {};

        if (!check_diagram_type<DiagramT>(d.type()))
            return {};

        if (roots_.empty())
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
        return std::any_of(matching_elements_.begin(), matching_elements_.end(),
            [&e](const auto &te) {
                return te.get().full_name(false) == e.full_name(false);
            });
    }

private:
    template <typename C, typename D>
    bool add_adjacent(const C &from, const D &to,
        const std::vector<relationship_t> &relationships) const
    {
        bool added_new_element{false};

        for (const auto &from_el : from) {
            //  Check if any of its relationships of type relationship_
            //  points to an element already in the matching_elements_
            //  set
            for (const auto &rel : from_el.get().relationships()) {
                // Consider only if connected by one of specified relationships
                if (util::contains(relationships, rel.type())) {
                    for (const auto &to_el : to) {
                        if (rel.destination() ==
                            detail::destination_comparator(to_el.get())) {
                            const auto &to_add = forward_ ? to_el : from_el;
                            if (matching_elements_.insert(to_add).second)
                                added_new_element = true;
                        }
                    }
                }
            }
        }

        return added_new_element;
    }

    void add_parents(const DiagramT &cd) const
    {
        decltype(matching_elements_) parents;

        util::for_each(
            matching_elements_, [&cd, &parents](const auto &element) {
                auto parent = detail::get<ElementT, DiagramT>(
                    cd, element.get().path().to_string());

                while (parent.has_value()) {
                    parents.emplace(parent.value());
                    parent = detail::get<ElementT, DiagramT>(
                        cd, parent.value().path().to_string());
                }
            });

        matching_elements_.insert(std::begin(parents), std::end(parents));
    }

    void init(const DiagramT &cd) const
    {
        if (initialized_)
            return;

        // First get all elements specified in the filter configuration
        // which will serve as starting points for the search
        // of matching elements
        for (const auto &template_root : roots_) {
            auto template_ref = detail::get<ElementT>(cd, template_root);
            if (template_ref.has_value()) {
                matching_elements_.emplace(template_ref.value());
            }
        }

        assert(roots_.empty() == matching_elements_.empty());

        bool keep_looking{true};
        while (keep_looking) {
            keep_looking = false;
            if (forward_) {
                if (add_adjacent(matching_elements_, detail::view<ElementT>(cd),
                        {relationship_}))
                    keep_looking = true;
            }
            else {
                if (add_adjacent(detail::view<ElementT>(cd), matching_elements_,
                        {relationship_}))
                    keep_looking = true;
            }
        }

        // For nested diagrams, include also parent elements
        if ((type() == filter_t::kInclusive) &&
            (cd.type() == common::model::diagram_t::kPackage)) {
            add_parents(cd);
        }

        initialized_ = true;
    }

    std::vector<std::string> roots_;
    relationship_t relationship_;
    mutable bool initialized_{false};
    mutable clanguml::common::reference_set<ElementT> matching_elements_;
    bool forward_;
};

struct relationship_filter : public filter_visitor {
    relationship_filter(
        filter_t type, std::vector<relationship_t> relationships);

    ~relationship_filter() override = default;

    tvl::value_t match(
        const diagram &d, const relationship_t &r) const override;

private:
    std::vector<relationship_t> relationships_;
};

struct access_filter : public filter_visitor {
    access_filter(filter_t type, std::vector<access_t> access);

    ~access_filter() override = default;

    tvl::value_t match(const diagram &d, const access_t &a) const override;

private:
    std::vector<access_t> access_;
};

struct context_filter : public filter_visitor {
    context_filter(filter_t type, std::vector<std::string> context);

    ~context_filter() override = default;

    tvl::value_t match(const diagram &d, const element &r) const override;

private:
    std::vector<std::string> context_;
};

struct paths_filter : public filter_visitor {
    paths_filter(filter_t type, const std::filesystem::path &root,
        const std::vector<std::filesystem::path> &p);

    ~paths_filter() override = default;

    tvl::value_t match(
        const diagram &d, const common::model::source_file &r) const override;

    tvl::value_t match(const diagram &d,
        const common::model::source_location &sl) const override;

private:
    std::vector<std::filesystem::path> paths_;
    std::filesystem::path root_;
};

struct class_method_filter : public filter_visitor {
    class_method_filter(filter_t type, std::unique_ptr<access_filter> af,
        std::unique_ptr<method_type_filter> mtf);

    ~class_method_filter() override = default;

    tvl::value_t match(const diagram &d,
        const class_diagram::model::class_method &m) const override;

private:
    std::unique_ptr<access_filter> access_filter_;
    std::unique_ptr<method_type_filter> method_type_filter_;
};

struct class_member_filter : public filter_visitor {
    class_member_filter(filter_t type, std::unique_ptr<access_filter> af);

    ~class_member_filter() override = default;

    tvl::value_t match(const diagram &d,
        const class_diagram::model::class_member &m) const override;

private:
    std::unique_ptr<access_filter> access_filter_;
};

class diagram_filter {
public:
    diagram_filter(const common::model::diagram &d, const config::diagram &c);

    void add_inclusive_filter(std::unique_ptr<filter_visitor> fv);

    void add_exclusive_filter(std::unique_ptr<filter_visitor> fv);

    bool should_include(const namespace_ &ns, const std::string &name) const;

    template <typename T> bool should_include(const T &e) const
    {
        auto exc = tvl::any_of(exclusive_.begin(), exclusive_.end(),
            [this, &e](const auto &ex) { return ex->match(diagram_, e); });

        if (tvl::is_true(exc))
            return false;

        auto inc = tvl::all_of(inclusive_.begin(), inclusive_.end(),
            [this, &e](const auto &in) { return in->match(diagram_, e); });

        return static_cast<bool>(tvl::is_undefined(inc) || tvl::is_true(inc));
    }

private:
    void init_filters(const config::diagram &c);

    std::vector<std::unique_ptr<filter_visitor>> inclusive_;
    std::vector<std::unique_ptr<filter_visitor>> exclusive_;

    const common::model::diagram &diagram_;
};

template <>
bool diagram_filter::should_include<std::string>(const std::string &name) const;
} // namespace clanguml::common::model