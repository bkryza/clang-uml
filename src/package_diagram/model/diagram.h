/**
 * @file src/package_diagram/model/diagram.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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
                public clanguml::common::model::element_views<package> {
public:
    diagram(const config::package_diagram &config)
        : clanguml::common::model::diagram{config}
        , config_{config}
    {
    }
    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = delete;

    const config::package_diagram &config() const { return config_; }

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
     * @brief Get alias of existing diagram element
     *
     * @param id Id of a package in the diagram
     * @return PlantUML alias of the element
     */
    std::string to_alias(eid_t id) const;

    /**
     * @brief Check whether the diagram is empty
     *
     * @return True, if diagram is empty
     */
    bool is_empty() const override;

    void apply_filter() override;

    /**
     * @brief Get reference to vector of elements of specific type
     *
     * @tparam ElementT Type of elements view
     * @return Reference to elements vector
     */
    template <typename ElementT>
    const common::reference_vector<ElementT> &elements() const;

    void append(diagram &&other);

private:

    const config::package_diagram &config_;
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
const common::reference_vector<ElementT> &diagram::elements() const
{
    return element_view<ElementT>::view();
}
} // namespace clanguml::package_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::package_diagram::model::diagram>(diagram_t t);
}
