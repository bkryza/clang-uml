/**
 * @file src/include_diagram/model/diagram.h
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

#include "common/model/diagram.h"
#include "common/model/element_view.h"
#include "common/model/package.h"
#include "common/model/source_file.h"
#include "common/types.h"

#include <string>
#include <vector>

namespace clanguml::include_diagram::model {

using clanguml::common::opt_ref;
using clanguml::common::model::diagram_element;
using clanguml::common::model::source_file;

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

    common::model::diagram_t type() const override;

    opt_ref<diagram_element> get(const std::string &full_name) const override;

    opt_ref<diagram_element> get(diagram_element::id_t id) const override;

    void add_file(std::unique_ptr<common::model::source_file> &&f);

    template <typename ElementT>
    opt_ref<ElementT> find(const std::string &name) const;

    template <typename ElementT>
    opt_ref<ElementT> find(diagram_element::id_t id) const;

    std::string to_alias(const std::string &full_name) const;

    const common::reference_vector<source_file> &files() const;

    opt_ref<diagram_element> get_with_namespace(const std::string &name,
        const common::model::namespace_ &ns) const override;

    inja::json context() const override;
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

template <typename ElementT>
opt_ref<ElementT> diagram::find(diagram_element::id_t id) const
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