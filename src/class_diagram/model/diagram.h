/**
 * src/class_diagram/model/diagram.h
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

#include "class.h"
#include "common/model/diagram.h"
#include "common/model/nested_trait.h"
#include "common/model/package.h"
#include "common/types.h"
#include "concept.h"
#include "enum.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace clanguml::class_diagram::model {

using common::opt_ref;
using common::model::diagram_element;
using common::model::diagram_t;

using nested_trait_ns =
    clanguml::common::model::nested_trait<clanguml::common::model::element,
        clanguml::common::model::namespace_>;

class diagram : public common::model::diagram::diagram, public nested_trait_ns {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    diagram_t type() const override;

    opt_ref<diagram_element> get(const std::string &full_name) const override;

    opt_ref<diagram_element> get(diagram_element::id_t id) const override;

    const common::reference_vector<class_> &classes() const;

    const common::reference_vector<enum_> &enums() const;

    const common::reference_vector<concept_> &concepts() const;

    bool has_class(const class_ &c) const;

    bool has_enum(const enum_ &e) const;

    bool has_concept(const concept_ &e) const;

    opt_ref<class_> get_class(const std::string &name) const;

    opt_ref<class_> get_class(diagram_element::id_t id) const;

    opt_ref<enum_> get_enum(const std::string &name) const;

    opt_ref<enum_> get_enum(diagram_element::id_t id) const;

    opt_ref<concept_> get_concept(const std::string &name) const;

    opt_ref<concept_> get_concept(diagram_element::id_t id) const;

    bool add_class_fs(
        const common::model::path &p, std::unique_ptr<class_> &&c);

    bool add_class(std::unique_ptr<class_> &&c);

    bool add_enum(std::unique_ptr<enum_> &&e);

    bool add_concept(std::unique_ptr<concept_> &&e);

    bool add_package(std::unique_ptr<common::model::package> &&p);

    bool add_package_fs(std::unique_ptr<common::model::package> &&p);

    std::string to_alias(diagram_element::id_t id) const;

    void get_parents(clanguml::common::reference_set<class_> &parents) const;

    friend void print_diagram_tree(const diagram &d, int level);

    bool has_element(diagram_element::id_t id) const override;

    inja::json context() const override;

private:
    common::reference_vector<class_> classes_;

    common::reference_vector<enum_> enums_;

    common::reference_vector<concept_> concepts_;
};
} // namespace clanguml::class_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::class_diagram::model::diagram>(diagram_t t);
}