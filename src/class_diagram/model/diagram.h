/**
 * src/class_diagram/model/diagram.h
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

#include "class.h"
#include "common/model/diagram.h"
#include "common/model/nested_trait.h"
#include "common/model/package.h"
#include "enum.h"
#include "type_alias.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace clanguml::class_diagram::model {

class diagram : public clanguml::common::model::diagram,
                public clanguml::common::model::nested_trait<
                    clanguml::common::model::element> {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    common::model::diagram_t type() const override;

    type_safe::optional_ref<const clanguml::common::model::element> get(
        const std::string &full_name) const override;

    const std::vector<type_safe::object_ref<const class_>> classes() const;

    const std::vector<type_safe::object_ref<const enum_>> enums() const;

    bool has_class(const class_ &c) const;

    bool has_enum(const enum_ &e) const;

    type_safe::optional_ref<const class_> get_class(
        const std::string &name) const;

    type_safe::optional_ref<const enum_> get_enum(
        const std::string &name) const;

    void add_type_alias(std::unique_ptr<type_alias> &&ta);

    void add_class(std::unique_ptr<class_> &&c);

    void add_enum(std::unique_ptr<enum_> &&e);

    void add_package(std::unique_ptr<common::model::package> &&p);

    std::string to_alias(const std::string &full_name) const;

    void get_parents(
        std::unordered_set<type_safe::object_ref<const class_>> &parents) const;

    friend void print_diagram_tree(const diagram &d, const int level);

private:
    std::vector<type_safe::object_ref<const class_, false>> classes_;
    std::vector<type_safe::object_ref<const enum_, false>> enums_;
    std::map<std::string, std::unique_ptr<type_alias>> type_aliases_;
};
}

namespace std {

template <>
struct hash<
    type_safe::object_ref<const clanguml::class_diagram::model::class_>> {
    std::size_t operator()(const type_safe::object_ref<
        const clanguml::class_diagram::model::class_> &key) const
    {
        using clanguml::class_diagram::model::class_;

        return std::hash<std::string>{}(key.get().full_name(false));
    }
};
}
