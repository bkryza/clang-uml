/**
 * @file src/include_diagram/model/diagram.h
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
#include "common/model/source_file.h"
#include "common/types.h"

#include <string>
#include <vector>

namespace clanguml::include_diagram::model {

using clanguml::common::eid_t;
using clanguml::common::opt_ref;
using clanguml::common::model::diagram_element;
using clanguml::common::model::source_file;

/**
 * @brief Class representing an include diagram model.
 */
class diagram : public clanguml::common::model::diagram,
                public clanguml::common::model::element_view<source_file>,
                public clanguml::common::model::nested_trait<source_file,
                    clanguml::common::model::filesystem_path> {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    /**
     * @brief Get the diagram model type - in this case include.
     *
     * @return Type of include diagram.
     */
    common::model::diagram_t type() const override;

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
     * @brief Add include diagram element, an include file.
     *
     * @param f Include diagram element
     */
    void add_file(std::unique_ptr<common::model::source_file> &&f);

    /**
     * @brief Find an element in the diagram by name.
     *
     * This method allows for typed search, where the type of searched for
     * element is determined from `ElementT`.
     *
     * @tparam ElementT Type of element (e.g. source_file)
     * @param name Fully qualified name of the element
     * @return Optional reference to a diagram element
     */
    template <typename ElementT>
    opt_ref<ElementT> find(const std::string &name) const;

    /**
     * @brief Find an element in the diagram by id.
     *
     * This method allows for typed search, where the type of searched for
     * element is determined from `ElementT`.
     *
     * @tparam ElementT Type of element (e.g. source_file)
     * @param id Id of the element
     * @return Optional reference to a diagram element
     */
    template <typename ElementT> opt_ref<ElementT> find(eid_t id) const;

    /**
     * @brief Get list of references to files in the diagram model.
     *
     * @return List of references to concepts in the diagram model.
     */
    const common::reference_vector<source_file> &files() const;

    /**
     * @brief Find diagram element with a specified name and path.
     *
     * @param name Name of the element
     * @param ns Path relative to the diagram
     * @return Optional reference to diagram element, if found.
     */
    opt_ref<diagram_element> get_with_namespace(const std::string &name,
        const common::model::namespace_ &ns) const override;

    inja::json context() const override;

    /**
     * @brief Check whether the diagram is empty
     *
     * @return True, if diagram is empty
     */
    bool is_empty() const override;
};

template <typename ElementT>
opt_ref<ElementT> diagram::find(const std::string &name) const
{
    // Convert the name to the OS preferred path
    std::filesystem::path namePath{name};
    namePath.make_preferred();

    for (const auto &element : element_view<ElementT>::view()) {
        const auto full_name = element.get().full_name(false);

        if (full_name == namePath.string()) {
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

} // namespace clanguml::include_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::include_diagram::model::diagram>(diagram_t t);
}