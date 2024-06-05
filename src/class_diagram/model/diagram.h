/**
 * @file src/class_diagram/model/diagram.h
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

#include "class.h"
#include "common/model/diagram.h"
#include "common/model/element_view.h"
#include "common/model/nested_trait.h"
#include "common/model/package.h"
#include "common/types.h"
#include "concept.h"
#include "config/config.h"
#include "enum.h"

#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

namespace clanguml::class_diagram::model {

using common::opt_ref;
using common::model::diagram_element;
using common::model::diagram_t;
using common::model::element_view;
using common::model::path;
using common::model::path_type;

using nested_trait_ns =
    clanguml::common::model::nested_trait<clanguml::common::model::element,
        clanguml::common::model::namespace_>;

/**
 * @brief Class representing a class diagram.
 */
class diagram : public common::model::diagram,
                public element_view<class_>,
                public element_view<enum_>,
                public element_view<concept_>,
                public nested_trait_ns {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    /**
     * @brief Get the diagram model type - in this case class.
     *
     * @return Type of class diagram.
     */
    diagram_t type() const override;

    /**
     * Inherit the should_include methods from the common diagram model.
     */
    using common::model::diagram::should_include;

    /**
     * @brief Whether a class_member should be included in the diagram.
     *
     * @param m Class member
     * @return True, if class member should be included in the diagram.
     */
    bool should_include(const class_member &m) const;

    /**
     * @brief Whether a class_method should be included in the diagram.
     *
     * @param m Class method
     * @return True, if class method should be included in the diagram.
     */
    bool should_include(const class_method &m) const;

    /**
     * @brief Search for element in the diagram by fully qualified name.
     *
     * @param full_name Fully qualified element name.
     * @return Optional reference to a diagram element.
     */
    opt_ref<diagram_element> get(const std::string &full_name) const override;

    /**
     * @brief Search for element in the diagram by id.
     *
     * @param id Element id.
     * @return Optional reference to a diagram element.
     */
    opt_ref<diagram_element> get(eid_t id) const override;

    /**
     * @brief Get list of references to classes in the diagram model.
     *
     * @return List of references to classes in the diagram model.
     */
    const common::reference_vector<class_> &classes() const;

    /**
     * @brief Get list of references to enums in the diagram model.
     *
     * @return List of references to enums in the diagram model.
     */
    const common::reference_vector<enum_> &enums() const;

    /**
     * @brief Get list of references to concepts in the diagram model.
     *
     * @return List of references to concepts in the diagram model.
     */
    const common::reference_vector<concept_> &concepts() const;

    /**
     * @brief Check, if diagram contains a specific element.
     *
     * @tparam ElementT Type of diagram element (e.g. class_)
     * @param e Element to check
     * @return True, if element already exists in the diagram
     */
    template <typename ElementT> bool contains(const ElementT &e);

    /**
     * @brief Find an element in the diagram by name.
     *
     * This method allows for typed search, where the type of searched for
     * element is determined from template specialization.
     *
     * @tparam ElementT Type of element (e.g. class_)
     * @param name Fully qualified name of the element
     * @return Optional reference to a diagram element
     */
    template <typename ElementT>
    opt_ref<ElementT> find(const std::string &name) const;

    /**
     * @brief Find elements in the diagram by regex pattern.
     *
     * This method allows for typed search, where the type of searched for
     * element is determined from template specialization.
     *
     * @tparam ElementT Type of element (e.g. class_)
     * @param name String or regex pattern
     * @return List of optional references to matched elements.
     */
    template <typename ElementT>
    std::vector<opt_ref<ElementT>> find(
        const clanguml::common::string_or_regex &pattern) const;

    /**
     * @brief Find an element in the diagram by id.
     *
     * This method allows for typed search, where the type of searched for
     * element is determined from template specialization.
     *
     * @tparam ElementT Type of element (e.g. class_)
     * @param id Id of the element
     * @return Optional reference to a diagram element
     */
    template <typename ElementT> opt_ref<ElementT> find(eid_t id) const;

    /**
     * @brief Get reference to vector of elements of specific type
     *
     * @tparam ElementT Type of elements view
     * @return Reference to elements vector
     */
    template <typename ElementT>
    const common::reference_vector<ElementT> &elements() const;

    /**
     * @brief Add element to the diagram at a specified nested path.
     *
     * Adds an element to a diagram, at a specific package (if any exist).
     * The package is specified by the `parent_path`, which can be either
     * a namespace or a directory path.
     *
     * @tparam ElementT Type of diagram element.
     * @param parent_path Path to the parent package of the new diagram element.
     * @param e Diagram element to be added.
     * @return True, if the element was added to the diagram.
     */
    template <typename ElementT>
    bool add(const path &parent_path, std::unique_ptr<ElementT> &&e)
    {
        if (parent_path.type() == common::model::path_type::kNamespace) {
            return add_with_namespace_path(std::move(e));
        }

        if (parent_path.type() == common::model::path_type::kModule) {
            return add_with_module_path(parent_path, std::move(e));
        }

        return add_with_filesystem_path(parent_path, std::move(e));
    }

    /**
     * @brief Convert element id to PlantUML alias.
     *
     * @todo This method does not belong here - refactor to PlantUML specific
     *       code.
     *
     * @param id Id of the diagram element.
     * @return PlantUML alias.
     */
    std::string to_alias(eid_t id) const;

    /**
     * @brief Given an initial set of classes, add all their parents to the
     *        argument.
     * @param parents In and out parameter with the parent classes.
     */
    void get_parents(clanguml::common::reference_set<class_> &parents) const;

    /**
     * @brief Check if diagram contains element by id.
     *
     * @todo Remove in favour of 'contains'
     *
     * @param id Id of the element.
     * @return True, if diagram contains an element with a specific id.
     */
    bool has_element(eid_t id) const override;

    /**
     * @brief Remove redundant dependency relationships
     */
    void remove_redundant_dependencies();

    /**
     * @brief Return the elements JSON context for inja templates.
     *
     * @return JSON node with elements context.
     */
    inja::json context() const override;

    /**
     * @brief Check whether the diagram is empty
     *
     * @return True, if diagram is empty
     */
    bool is_empty() const override;

private:
    template <typename ElementT>
    bool add_with_namespace_path(std::unique_ptr<ElementT> &&e);

    template <typename ElementT>
    bool add_with_module_path(
        const common::model::path &parent_path, std::unique_ptr<ElementT> &&e);

    template <typename ElementT>
    bool add_with_filesystem_path(
        const common::model::path &parent_path, std::unique_ptr<ElementT> &&e);
};

template <typename ElementT> bool diagram::contains(const ElementT &element)
{
    return std::any_of(element_view<ElementT>::view().cbegin(),
        element_view<ElementT>::view().cend(),
        [&element](
            const auto &element_opt) { return element_opt.get() == element; });
}

template <typename ElementT>
bool diagram::add_with_namespace_path(std::unique_ptr<ElementT> &&e)
{
    const auto base_name = e->name();
    const auto full_name = e->full_name(false);
    const auto element_type = e->type_name();

    LOG_DBG("Adding {}: {}::{}, {}", element_type,
        e->get_namespace().to_string(), base_name, full_name);

    if (util::contains(base_name, "::"))
        throw std::runtime_error("Name cannot contain namespace: " + base_name);

    if (util::contains(base_name, "*"))
        throw std::runtime_error("Name cannot contain *: " + base_name);

    const auto ns = e->get_relative_namespace();
    auto name = base_name;
    auto name_with_ns = e->name_and_ns();
    auto name_and_ns = ns | name;
    auto &e_ref = *e;
    auto id = e_ref.id();

    try {
        if (!contains(e_ref)) {
            if (add_element(ns, std::move(e)))
                element_view<ElementT>::add(std::ref(e_ref));

            const auto &el = get_element<ElementT>(name_and_ns).value();

            if ((el.name() != name) || !(el.get_relative_namespace() == ns))
                throw std::runtime_error(
                    "Invalid element stored in the diagram tree");

            LOG_DBG("Added {} {} ({} - [{}])", element_type, base_name,
                full_name, id);

            return true;
        }
    }
    catch (const std::runtime_error &e) {
        LOG_WARN("Cannot add {} {} with id {} due to: {}", element_type, name,
            id, e.what());
        return false;
    }

    LOG_DBG("{} {} ({} - [{}]) already in the model", element_type, base_name,
        full_name, id);

    return false;
}

template <typename ElementT>
bool diagram::add_with_module_path(
    const common::model::path &parent_path, std::unique_ptr<ElementT> &&e)
{
    const auto element_type = e->type_name();

    // Make sure all parent modules are already packages in the
    // model
    for (auto it = parent_path.begin(); it != parent_path.end(); it++) {
        auto pkg = std::make_unique<common::model::package>(
            e->using_namespace(), parent_path.type());
        pkg->set_name(*it);
        auto ns =
            common::model::path(parent_path.begin(), it, parent_path.type());
        // ns.pop_back();
        pkg->set_namespace(ns);
        pkg->set_id(common::to_id(pkg->full_name(false)));

        add(ns, std::move(pkg));
    }

    const auto base_name = e->name();
    const auto full_name = e->full_name(false);
    auto &e_ref = *e;

    if (add_element(parent_path, std::move(e))) {
        element_view<ElementT>::add(std::ref(e_ref));
        return true;
    }

    return false;
}

template <typename ElementT>
bool diagram::add_with_filesystem_path(
    const common::model::path &parent_path, std::unique_ptr<ElementT> &&e)
{
    const auto element_type = e->type_name();

    // Make sure all parent modules are already packages in the
    // model
    for (auto it = parent_path.begin(); it != parent_path.end(); it++) {
        auto pkg = std::make_unique<common::model::package>(
            e->using_namespace(), parent_path.type());
        pkg->set_name(*it);
        auto ns =
            common::model::path(parent_path.begin(), it, parent_path.type());
        // ns.pop_back();
        pkg->set_namespace(ns);
        pkg->set_id(common::to_id(pkg->full_name(false)));

        add(ns, std::move(pkg));
    }

    const auto base_name = e->name();
    const auto full_name = e->full_name(false);
    auto &e_ref = *e;

    if (add_element(parent_path, std::move(e))) {
        element_view<ElementT>::add(std::ref(e_ref));
        return true;
    }

    return false;
}

template <typename ElementT>
opt_ref<ElementT> diagram::find(const std::string &name) const
{
    for (const auto &element : element_view<ElementT>::view()) {
        const auto full_name = element.get().full_name(false);

        auto full_name_escaped = full_name;
        util::replace_all(full_name_escaped, "##", "::");

        if (name == full_name || name == full_name_escaped) {
            return {element};
        }
    }

    return {};
}

template <typename ElementT>
std::vector<opt_ref<ElementT>> diagram::find(
    const common::string_or_regex &pattern) const
{
    std::vector<opt_ref<ElementT>> result;

    for (const auto &element : element_view<ElementT>::view()) {
        const auto full_name = element.get().full_name(false);
        auto full_name_escaped = full_name;
        util::replace_all(full_name_escaped, "##", "::");

        if (pattern == full_name || pattern == full_name_escaped) {
            result.emplace_back(element);
        }
    }

    return result;
}

template <typename ElementT> opt_ref<ElementT> diagram::find(eid_t id) const
{
    for (const auto &element : element_view<ElementT>::view()) {
        if (element.get().id() == id) {
            return {element};
        }
    }

    return {};
}

template <typename ElementT>
const common::reference_vector<ElementT> &diagram::elements() const
{
    return element_view<ElementT>::view();
}

//
// Template method specialization pre-declarations...
//
template <>
bool diagram::add_with_namespace_path<common::model::package>(
    std::unique_ptr<common::model::package> &&p);

template <>
bool diagram::add_with_module_path<common::model::package>(
    const common::model::path &parent_path,
    std::unique_ptr<common::model::package> &&p);

template <>
bool diagram::add_with_filesystem_path<common::model::package>(
    const common::model::path &parent_path,
    std::unique_ptr<common::model::package> &&p);

} // namespace clanguml::class_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::class_diagram::model::diagram>(diagram_t t);
}