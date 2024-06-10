/**
 * @file src/common/model/diagram_filter.h
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
#include "sequence_diagram/model/participant.h"
#include "source_file.h"
#include "tvl.h"

#include <filesystem>
#include <utility>

namespace clanguml::common::model {

using clanguml::common::eid_t;

/**
 * Diagram filters can be add in 2 modes:
 *  - inclusive - the elements that match are included in the diagram
 *  - exclusive - the elements that match are excluded from the diagram
 */
enum class filter_t {
    kInclusive, /*!< Filter is inclusive */
    kExclusive  /*!< Filter is exclusve */
};

namespace detail {
template <typename ElementT, typename DiagramT>
const clanguml::common::reference_vector<ElementT> &view(const DiagramT &d);

template <typename ElementT, typename DiagramT>
const clanguml::common::optional_ref<ElementT> get(
    const DiagramT &d, const std::string &full_name);

template <typename ElementT> eid_t destination_comparator(const ElementT &e)
{
    return e.id();
}

template <> eid_t destination_comparator(const common::model::source_file &f);
} // namespace detail

/**
 * @brief Base class for any diagram filter.
 *
 * This class acts as a visitor for diagram elements. It provides a set of
 * common methods which can be overriden by specific filters. If a filter
 * does not implement a specific method, it is ignored through the 3 value
 * logic implemented in @see clanguml::common::model::tvl
 *
 * @embed{filter_visitor_hierarchy_class.svg}
 */
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

    virtual tvl::value_t match(
        const diagram &d, const sequence_diagram::model::participant &p) const;

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

    tvl::value_t match(const diagram &d,
        const sequence_diagram::model::participant &p) const override;

    tvl::value_t match(
        const diagram &d, const common::model::source_file &e) const override;

private:
    std::vector<std::unique_ptr<filter_visitor>> filters_;
};

/**
 * Match namespace or diagram element to a set of specified namespaces or
 * regex patterns.
 */
struct namespace_filter : public filter_visitor {
    namespace_filter(
        filter_t type, std::vector<common::namespace_or_regex> namespaces);

    ~namespace_filter() override = default;

    tvl::value_t match(const diagram &d, const namespace_ &ns) const override;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<common::namespace_or_regex> namespaces_;
};

/**
 * Match diagram elements to a set of specified modules or
 * module regex patterns.
 */
struct modules_filter : public filter_visitor {
    modules_filter(filter_t type, std::vector<common::string_or_regex> modules);

    ~modules_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<common::string_or_regex> modules_;
};

/**
 * Match element's name to a set of names or regex patterns.
 */
struct element_filter : public filter_visitor {
    element_filter(
        filter_t type, std::vector<common::string_or_regex> elements);

    ~element_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

    tvl::value_t match(const diagram &d,
        const sequence_diagram::model::participant &p) const override;

private:
    std::vector<common::string_or_regex> elements_;
};

/**
 * Match diagram elements based on elements type (e.g. class).
 */
struct element_type_filter : public filter_visitor {
    element_type_filter(filter_t type, std::vector<std::string> element_types);

    ~element_type_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<std::string> element_types_;
};

/**
 * Match class methods based on their category (e.g. operator).
 */
struct method_type_filter : public filter_visitor {
    method_type_filter(
        filter_t type, std::vector<config::method_type> method_types);

    ~method_type_filter() override = default;

    tvl::value_t match(const diagram &d,
        const class_diagram::model::class_method &e) const override;

private:
    std::vector<config::method_type> method_types_;
};

/**
 * Sequence diagram callee type filter.
 */
struct callee_filter : public filter_visitor {
    callee_filter(filter_t type, std::vector<config::callee_type> callee_types);

    ~callee_filter() override = default;

    tvl::value_t match(const diagram &d,
        const sequence_diagram::model::participant &p) const override;

private:
    std::vector<config::callee_type> callee_types_;
};

/**
 * Match element based on whether it is a subclass of a set of base classes,
 * or one of them.
 */
struct subclass_filter : public filter_visitor {
    subclass_filter(filter_t type, std::vector<common::string_or_regex> roots);

    ~subclass_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<common::string_or_regex> roots_;
};

/**
 * Match element based on whether it is a parent of a set of children, or one
 * of them.
 */
struct parents_filter : public filter_visitor {
    parents_filter(filter_t type, std::vector<common::string_or_regex> roots);

    ~parents_filter() override = default;

    tvl::value_t match(const diagram &d, const element &e) const override;

private:
    std::vector<common::string_or_regex> children_;
};

/**
 * @brief Common template for filters involving traversing relationship graph.
 *
 * This class template provides a common implementation of a diagram
 * relationship graph traversal. It is used for filters, which need to check
 * for instance, whether an element is in some kind of relationship with other
 * element.
 *
 * @tparam DiagramT Diagram type
 * @tparam ElementT Element type
 * @tparam ConfigEntryT Type of configuration option used to specify initial
 *                      elements for traversal
 * @tparam MatchOverrideT Type of the matched element
 */
template <typename DiagramT, typename ElementT,
    typename ConfigEntryT = std::string,
    typename MatchOverrideT = common::model::element>
struct edge_traversal_filter : public filter_visitor {
    edge_traversal_filter(filter_t type, relationship_t relationship,
        std::vector<ConfigEntryT> roots, bool forward = false)
        : filter_visitor{type}
        , roots_{std::move(roots)}
        , relationship_{relationship}
        , forward_{forward}
    {
    }

    ~edge_traversal_filter() override = default;

    tvl::value_t match(const diagram &d, const MatchOverrideT &e) const override
    {
        // This filter should only be run only on diagram models after the
        // entire AST has been visited
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
                std::string tes = te.get().full_name(false);
                std::string es = e.full_name(false);

                if (tes == es)
                    return true;

                return false;
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
        for (const auto &root_pattern : roots_) {
            if constexpr (std::is_same_v<ConfigEntryT,
                              common::string_or_regex>) {
                auto root_refs = cd.template find<ElementT>(root_pattern);

                for (auto &root : root_refs) {
                    if (root.has_value())
                        matching_elements_.emplace(root.value());
                }
            }
            else {
                auto root_ref = detail::get<ElementT>(cd, root_pattern);
                if (root_ref.has_value()) {
                    matching_elements_.emplace(root_ref.value());
                }
            }
        }

        bool keep_looking{!matching_elements_.empty()};
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

    std::vector<ConfigEntryT> roots_;
    relationship_t relationship_;
    mutable bool initialized_{false};
    mutable clanguml::common::reference_set<ElementT> matching_elements_;
    bool forward_;
};

/**
 * Match relationship types.
 */
struct relationship_filter : public filter_visitor {
    relationship_filter(
        filter_t type, std::vector<relationship_t> relationships);

    ~relationship_filter() override = default;

    tvl::value_t match(
        const diagram &d, const relationship_t &r) const override;

private:
    std::vector<relationship_t> relationships_;
};

/**
 * Match class members and methods based on access (public, protected, private).
 */
struct access_filter : public filter_visitor {
    access_filter(filter_t type, std::vector<access_t> access);

    ~access_filter() override = default;

    tvl::value_t match(const diagram &d, const access_t &a) const override;

private:
    std::vector<access_t> access_;
};

/**
 * Match diagram elements based on module access (public or private).
 */
struct module_access_filter : public filter_visitor {
    module_access_filter(filter_t type, std::vector<module_access_t> access);

    ~module_access_filter() override = default;

    tvl::value_t match(const diagram &d, const element &a) const override;

private:
    std::vector<module_access_t> access_;
};

/**
 * Match diagram elements which are in within a 'radius' distance relationship
 * to any of the elements specified in context.
 */
struct context_filter : public filter_visitor {
    context_filter(filter_t type, std::vector<config::context_config> context);

    ~context_filter() override = default;

    tvl::value_t match(const diagram &d, const element &r) const override;

private:
    void initialize(const diagram &d) const;

    void initialize_effective_context(const diagram &d, unsigned idx) const;

    template <typename ElementT>
    void find_elements_in_direct_relationship(const diagram &d,
        std::set<eid_t> &effective_context,
        std::set<eid_t> &current_iteration_context) const
    {
        static_assert(std::is_same_v<ElementT, class_diagram::model::class_> ||
                std::is_same_v<ElementT, class_diagram::model::enum_> ||
                std::is_same_v<ElementT, class_diagram::model::concept_>,
            "ElementT must be either class_ or enum_ or concept_");

        const auto &cd = dynamic_cast<const class_diagram::model::diagram &>(d);

        for (const auto &el : cd.elements<ElementT>()) {
            // First search all elements of type ElementT in the diagram
            // which have a relationship to any of the effective_context
            // elements
            for (const relationship &rel : el.get().relationships()) {
                for (const auto &element_id : effective_context) {
                    if (d.should_include(rel.type()) &&
                        rel.destination() == element_id)
                        current_iteration_context.emplace(el.get().id());
                }
            }

            // Now search current effective_context elements and add any
            // elements of any type in the diagram which to that element
            for (const auto element_id : effective_context) {
                const auto &maybe_element = cd.get(element_id);

                if (!maybe_element)
                    continue;

                for (const relationship &rel :
                    maybe_element.value().relationships()) {

                    if (d.should_include(rel.type()) &&
                        rel.destination() == el.get().id())
                        current_iteration_context.emplace(el.get().id());
                }
            }
        }
    }

    void find_elements_inheritance_relationship(const diagram &d,
        std::set<eid_t> &effective_context,
        std::set<eid_t> &current_iteration_context) const;

    std::vector<config::context_config> context_;

    /*!
     * Represents all elements which should belong to the diagram based
     * on this filter. It is populated by the initialize() method.
     */
    mutable std::vector<std::set<eid_t>> effective_contexts_;

    /*! Flag to mark whether the filter context has been computed */
    mutable bool initialized_{false};
};

/**
 * Match elements based on their source location, whether it matches to
 * a specified file paths.
 */
struct paths_filter : public filter_visitor {
    paths_filter(filter_t type, const std::filesystem::path &root,
        const std::vector<std::string> &p);

    ~paths_filter() override = default;

    tvl::value_t match(
        const diagram &d, const common::model::source_file &r) const override;

    tvl::value_t match(const diagram &d,
        const common::model::source_location &sl) const override;

private:
    std::vector<std::filesystem::path> paths_;
    std::filesystem::path root_;
};

/**
 * Match class method based on specified method categories.
 */
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

/**
 * Match class members.
 */
struct class_member_filter : public filter_visitor {
    class_member_filter(filter_t type, std::unique_ptr<access_filter> af);

    ~class_member_filter() override = default;

    tvl::value_t match(const diagram &d,
        const class_diagram::model::class_member &m) const override;

private:
    std::unique_ptr<access_filter> access_filter_;
};

/**
 * @brief Composite of all diagrams filters.
 *
 * Instances of this class contain all filters specified in configuration file
 * for a given diagram.
 *
 * @embed{diagram_filter_context_class.svg}
 *
 * @see clanguml::common::model::filter_visitor
 */
class diagram_filter {
public:
    diagram_filter(const common::model::diagram &d, const config::diagram &c);

    /**
     * Add inclusive filter.
     *
     * @param fv Filter visitor.
     */
    void add_inclusive_filter(std::unique_ptr<filter_visitor> fv);

    /** Add exclusive filter.
     *
     * @param fv Filter visitor.
     */
    void add_exclusive_filter(std::unique_ptr<filter_visitor> fv);

    /**
     * `should_include` overload for namespace and name.
     *
     * @param ns Namespace
     * @param name Name
     * @return Match result.
     */
    bool should_include(const namespace_ &ns, const std::string &name) const;

    /**
     * Generic `should_include` overload for various diagram elements.
     *
     * @tparam T Type to to match - must match one of filter_visitor's match(T)
     * @param e Value of type T to match
     * @return Match result.
     */
    template <typename T> bool should_include(const T &e) const
    {
        auto exc = tvl::any_of(
            exclusive_.begin(), exclusive_.end(), [this, &e](const auto &ex) {
                assert(ex.get() != nullptr);

                return ex->match(diagram_, e);
            });

        if (tvl::is_true(exc))
            return false;

        auto inc = tvl::all_of(
            inclusive_.begin(), inclusive_.end(), [this, &e](const auto &in) {
                assert(in.get() != nullptr);

                return in->match(diagram_, e);
            });

        return static_cast<bool>(tvl::is_undefined(inc) || tvl::is_true(inc));
    }

private:
    /**
     * @brief Initialize filters.
     *
     * Some filters require initialization.
     *
     * @param c Diagram config.
     */
    void init_filters(const config::diagram &c);

    /*! List of inclusive filters */
    std::vector<std::unique_ptr<filter_visitor>> inclusive_;

    /*! List of exclusive filters */
    std::vector<std::unique_ptr<filter_visitor>> exclusive_;

    /*! Reference to the diagram model */
    const common::model::diagram &diagram_;
};

template <>
bool diagram_filter::should_include<std::string>(const std::string &name) const;
} // namespace clanguml::common::model