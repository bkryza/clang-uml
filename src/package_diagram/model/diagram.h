/**
 * @file src/package_diagram/model/diagram.h
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

#include "common/model/diagram.h"
#include "common/model/element_view.h"
#include "common/model/package.h"

#include <string>
#include <vector>

namespace clanguml::package_diagram::model {

using clanguml::common::eid_t;
using clanguml::common::opt_ref;
using clanguml::common::model::diagram_element;
using clanguml::common::model::package;
using clanguml::common::model::path;

/**
 * @brief Package diagram model.
 */
class diagram : public clanguml::common::model::diagram,
                public clanguml::common::model::element_view<package>,
                public clanguml::common::model::nested_trait<
                    clanguml::common::model::element,
                    clanguml::common::model::namespace_> {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    /**
     * @brief Get the diagram model type - in this case package.
     *
     * @return Type of package diagram.
     */
    common::model::diagram_t type() const override;

    /**
     * @brief Get list of references to packages in the diagram model.
     *
     * @return List of references to packages in the diagram model.
     */
    const common::reference_vector<package> &packages() const;

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
     * @brief Find an element in the diagram by name.
     *
     * This method allows for typed search, where the type of searched for
     * element is determined from template specialization.
     *
     * @tparam ElementT Type of element (e.g. package)
     * @param name Fully qualified name of the element
     * @return Optional reference to a diagram element
     */
    template <typename ElementT>
    opt_ref<ElementT> find(const std::string &name) const;

    /**
     * @brief Find an element in the diagram by id.
     *
     * This method allows for typed search, where the type of searched for
     * element is determined from template specialization.
     *
     * @tparam ElementT Type of element (e.g. package)
     * @param id Id of the element
     * @return Optional reference to a diagram element
     */
    template <typename ElementT> opt_ref<ElementT> find(eid_t id) const;

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
     * @brief Add diagram element at nested path
     *
     * This method handled both diagrams where packages are created from
     * namespaces, as well as those were packages are created from project
     * subdirectories.
     *
     * @tparam ElementT Type of diagram element to add
     * @param parent_path Package nested path where the element should be added
     * @param e Diagram element to add
     * @return True, if the element was added.
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
     * @brief Get alias of existing diagram element
     *
     * @param id Id of a package in the diagram
     * @return PlantUML alias of the element
     */
    std::string to_alias(eid_t id) const;

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
    /**
     * @brief Add element using module as diagram path
     *
     * @tparam ElementT Element type
     * @param e Element to add
     * @return True, if the element was added
     */
    template <typename ElementT>
    bool add_with_module_path(
        const common::model::path &parent_path, std::unique_ptr<ElementT> &&e);

    /**
     * @brief Add element using namespace as diagram path
     *
     * @tparam ElementT Element type
     * @param e Element to add
     * @return True, if the element was added
     */
    template <typename ElementT>
    bool add_with_namespace_path(std::unique_ptr<ElementT> &&e);

    /**
     * @brief Add element using relative filesystem path as diagram path
     *
     * @tparam ElementT Element type
     * @param parent_path Path to diagram elements parent package
     * @param e Element to add
     * @return True, if the element was added
     */
    template <typename ElementT>
    bool add_with_filesystem_path(
        const common::model::path &parent_path, std::unique_ptr<ElementT> &&e);
};

template <typename ElementT>
opt_ref<ElementT> diagram::find(const std::string &name) const
{
    for (const auto &element : element_view<ElementT>::view()) {
        const auto full_name = element.get().full_name(false);

        if (full_name == name) {
            return {element};
        }
    }

    return {};
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
std::vector<opt_ref<ElementT>> diagram::find(
    const common::string_or_regex &pattern) const
{
    std::vector<opt_ref<ElementT>> result;

    for (const auto &element : element_view<ElementT>::view()) {
        const auto full_name = element.get().full_name(false);

        if (pattern == full_name) {
            result.emplace_back(element);
        }
    }

    return result;
}

template <typename ElementT>
bool diagram::add_with_namespace_path(std::unique_ptr<ElementT> &&p)
{
    LOG_DBG(
        "Adding package: {}, {}, [{}]", p->name(), p->full_name(true), p->id());

    auto ns = p->get_relative_namespace();
    auto p_ref = std::ref(*p);

    auto res = add_element(ns, std::move(p));
    if (res)
        element_view<ElementT>::add(p_ref);

    return res;
}

template <typename ElementT>
bool diagram::add_with_module_path(
    const common::model::path &parent_path, std::unique_ptr<ElementT> &&p)
{
    LOG_DBG("Adding package: {}, {}, {}, [{}]", p->name(), p->full_name(false),
        parent_path.to_string(), p->id());

    // Make sure all parent modules are already packages in the
    // model
    auto module_relative_to = path{p->using_namespace()};

    for (auto it = parent_path.begin(); it != parent_path.end(); it++) {
        auto pkg = std::make_unique<common::model::package>(
            p->using_namespace(), common::model::path_type::kModule);
        pkg->set_name(*it);

        auto module_relative_part = common::model::path(
            parent_path.begin(), it, common::model::path_type::kModule);

        auto module_absolute_path = module_relative_to | module_relative_part;
        pkg->set_module(module_absolute_path.to_string());
        pkg->set_namespace(module_absolute_path);

        auto package_absolute_path = module_absolute_path | pkg->name();

        pkg->set_id(common::to_id(package_absolute_path.to_string()));

        auto p_ref = std::ref(*pkg);

        auto res = add_element(module_relative_part, std::move(pkg));
        if (res)
            element_view<ElementT>::add(p_ref);
    }

    auto p_ref = std::ref(*p);

    auto res = add_element(parent_path, std::move(p));
    if (res)
        element_view<ElementT>::add(p_ref);

    return res;
}

template <typename ElementT>
bool diagram::add_with_filesystem_path(
    const common::model::path &parent_path, std::unique_ptr<ElementT> &&p)
{
    LOG_DBG("Adding package: {}, {}", p->name(), p->full_name(true));

    // Make sure all parent directories are already packages in the
    // model
    for (auto it = parent_path.begin(); it != parent_path.end(); it++) {
        auto pkg =
            std::make_unique<common::model::package>(p->using_namespace());
        pkg->set_name(*it);
        auto ns = common::model::path(parent_path.begin(), it);
        pkg->set_namespace(ns);
        pkg->set_id(common::to_id(pkg->full_name(false)));

        add_with_filesystem_path(ns, std::move(pkg));
    }

    auto pp = std::ref(*p);
    auto res = add_element(parent_path, std::move(p));
    if (res)
        element_view<ElementT>::add(pp);

    return res;
}

} // namespace clanguml::package_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::package_diagram::model::diagram>(diagram_t t);
}
