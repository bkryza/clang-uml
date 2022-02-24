/**
 * src/class_diagram/visitor/translation_unit_context.h
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

#include "config/config.h"

#include <cppast/cpp_entity_index.hpp>
#include <cppast/cpp_namespace.hpp>
#include <cppast/cpp_type.hpp>
#include <type_safe/reference.hpp>

namespace clanguml::class_diagram::visitor {

class translation_unit_context {
public:
    translation_unit_context(cppast::cpp_entity_index &idx,
        clanguml::class_diagram::model::diagram &diagram,
        const clanguml::config::class_diagram &config);

    bool has_namespace_alias(const std::string &full_name) const;

    void add_namespace_alias(const std::string &full_name,
        type_safe::object_ref<const cppast::cpp_namespace> ref);

    type_safe::object_ref<const cppast::cpp_namespace> get_namespace_alias(
        const std::string &full_name) const;

    type_safe::object_ref<const cppast::cpp_namespace>
    get_namespace_alias_final(const cppast::cpp_namespace &t) const;

    bool has_type_alias(const std::string &full_name) const;

    void add_type_alias(const std::string &full_name,
        type_safe::object_ref<const cppast::cpp_type> &&ref);

    type_safe::object_ref<const cppast::cpp_type> get_type_alias(
        const std::string &full_name) const;

    type_safe::object_ref<const cppast::cpp_type> get_type_alias_final(
        const cppast::cpp_type &t) const;

    bool has_type_alias_template(const std::string &full_name) const;

    void add_type_alias_template(const std::string &full_name,
        type_safe::object_ref<const cppast::cpp_type> &&ref);

    type_safe::object_ref<const cppast::cpp_type> get_type_alias_template(
        const std::string &full_name) const;

    void push_namespace(const std::string &ns);

    void pop_namespace();

    const std::vector<std::string> &get_namespace() const;

    const cppast::cpp_entity_index &entity_index() const;

    const clanguml::config::class_diagram &config() const;

    clanguml::class_diagram::model::diagram &diagram();

    void set_current_package(type_safe::optional_ref<common::model::package> p);

    type_safe::optional_ref<common::model::package> get_current_package() const;

private:
    // Current visitor namespace
    std::vector<std::string> namespace_;

    // Reference to the cppast entity index
    cppast::cpp_entity_index &entity_index_;

    // Reference to the output diagram model
    clanguml::class_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::class_diagram &config_;

    // Map of discovered aliases (declared with 'namespace' keyword)
    std::map<std::string, type_safe::object_ref<const cppast::cpp_namespace>>
        namespace_alias_index_;

    // Map of discovered aliases (declared with 'using' keyword)
    std::map<std::string, type_safe::object_ref<const cppast::cpp_type>>
        alias_index_;

    // Map of discovered template aliases (declared with 'using' keyword)
    std::map<std::string, type_safe::object_ref<const cppast::cpp_type>>
        alias_template_index_;

    type_safe::optional_ref<common::model::package> current_package_;
};

}
