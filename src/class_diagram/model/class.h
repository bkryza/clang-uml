/**
 * src/class_diagram/model/class.h
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

#include "class_member.h"
#include "class_method.h"
#include "class_parent.h"
#include "common/model/element.h"
#include "common/model/enums.h"
#include "common/model/stylable_element.h"
#include "common/types.h"
#include "template_parameter.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

class class_ : public common::model::element,
               public common::model::stylable_element {
public:
    class_(const common::model::namespace_ &using_namespace);

    class_(const class_ &) = delete;
    class_(class_ &&) noexcept = delete;
    class_ &operator=(const class_ &) = delete;
    class_ &operator=(class_ &&) = delete;

    std::string type_name() const override { return "class"; }

    bool is_struct() const;
    void is_struct(bool is_struct);

    bool is_template() const;
    void is_template(bool is_template);

    bool is_template_instantiation() const;
    void is_template_instantiation(bool is_template_instantiation);

    void add_member(class_member &&member);
    void add_method(class_method &&method);
    void add_parent(class_parent &&parent);
    void add_template(template_parameter &&tmplt);

    const std::vector<class_member> &members() const;
    const std::vector<class_method> &methods() const;
    const std::vector<class_parent> &parents() const;
    const std::vector<template_parameter> &templates() const;

    void set_base_template(const std::string &full_name);
    std::string base_template() const;

    friend bool operator==(const class_ &l, const class_ &r);

    std::string full_name(bool relative = true) const override;

    std::string full_name_no_ns() const override;

    bool is_abstract() const;

    bool is_alias() const { return is_alias_; }

    void is_alias(bool alias) { is_alias_ = alias; }

    void find_relationships(
        std::vector<std::pair<std::string, common::model::relationship_t>>
            &nested_relationships);

    int calculate_template_specialization_match(
        const class_ &other, const std::string &full_name) const;

private:
    std::ostringstream &render_template_params(
        std::ostringstream &ostr, bool relative) const;

    bool is_struct_{false};
    bool is_template_{false};
    bool is_template_instantiation_{false};
    bool is_alias_{false};
    std::vector<class_member> members_;
    std::vector<class_method> methods_;
    std::vector<class_parent> bases_;
    std::vector<template_parameter> templates_;
    std::string base_template_full_name_;

    std::string full_name_;
};

} // namespace clanguml::class_diagram::model

namespace std {
template <>
struct hash<std::reference_wrapper<clanguml::class_diagram::model::class_>> {
    std::size_t operator()(
        const std::reference_wrapper<clanguml::class_diagram::model::class_>
            &key) const
    {
        using clanguml::common::id_t;

        return std::hash<id_t>{}(key.get().id());
    }
};
} // namespace std
