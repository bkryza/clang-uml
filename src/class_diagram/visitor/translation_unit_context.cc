/**
 * src/class_diagram/visitor/translation_unit_context.cc
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

#include "translation_unit_context.h"

#include "cx/util.h"

namespace clanguml::class_diagram::visitor {

translation_unit_context::translation_unit_context(
    cppast::cpp_entity_index &idx,
    clanguml::class_diagram::model::diagram &diagram,
    const clanguml::config::class_diagram &config)
    : entity_index_{idx}
    , diagram_{diagram}
    , config_{config}
{
}

bool translation_unit_context::has_namespace_alias(
    const std::string &full_name) const
{
    bool res =
        namespace_alias_index_.find(full_name) != namespace_alias_index_.end();

    LOG_DBG("Alias {} {} found in index", full_name, res ? "" : "not");

    return res;
}

void translation_unit_context::add_namespace_alias(const std::string &full_name,
    type_safe::object_ref<const cppast::cpp_namespace> ref)
{
    if (!has_namespace_alias(full_name)) {
        LOG_DBG(
            "Stored namespace alias: {} -> {} ", full_name, ref.get().name());

        namespace_alias_index_.emplace(full_name, std::move(ref));
    }
}

type_safe::object_ref<const cppast::cpp_namespace>
translation_unit_context::get_namespace_alias(
    const std::string &full_name) const
{
    assert(has_namespace_alias(full_name));

    return namespace_alias_index_.at(full_name);
}

type_safe::object_ref<const cppast::cpp_namespace>
translation_unit_context::get_namespace_alias_final(
    const cppast::cpp_namespace &ns) const
{
    auto ns_full_name = cx::util::full_name({}, ns);

    ns_full_name = cx::util::ns(ns) + "::" + ns_full_name;

    if (has_namespace_alias(ns_full_name)) {
        return get_namespace_alias_final(
            namespace_alias_index_.at(ns_full_name).get());
    }

    return type_safe::ref(ns);
}

bool translation_unit_context::has_type_alias(
    const std::string &full_name) const
{
    bool res = alias_index_.find(full_name) != alias_index_.end();

    LOG_DBG("Alias {} {} found in index", full_name, res ? "" : "not");

    return res;
}

void translation_unit_context::add_type_alias(const std::string &full_name,
    type_safe::object_ref<const cppast::cpp_type> &&ref)
{
    if (!has_type_alias(full_name)) {
        LOG_DBG("Stored type alias: {} -> {} ", full_name,
            cppast::to_string(ref.get()));

        alias_index_.emplace(full_name, std::move(ref));
    }
}

type_safe::object_ref<const cppast::cpp_type>
translation_unit_context::get_type_alias(const std::string &full_name) const
{
    assert(has_type_alias(full_name));

    return alias_index_.at(full_name);
}

type_safe::object_ref<const cppast::cpp_type>
translation_unit_context::get_type_alias_final(const cppast::cpp_type &t) const
{
    const auto type_full_name =
        cx::util::full_name(cppast::remove_cv(t), entity_index_, false);

    if (has_type_alias(type_full_name)) {
        const auto &alias_type = alias_index_.at(type_full_name).get();
        // Prevent infinite recursion
        if (type_full_name !=
            cx::util::full_name(
                cppast::remove_cv(alias_type), entity_index_, false))
            return get_type_alias_final(alias_type);
    }

    return type_safe::ref(t);
}

bool translation_unit_context::has_type_alias_template(
    const std::string &full_name) const
{
    bool res =
        alias_template_index_.find(full_name) != alias_template_index_.end();

    LOG_DBG("Alias template {} {} found in index", full_name, res ? "" : "not");

    return res;
}

void translation_unit_context::add_type_alias_template(
    const std::string &full_name,
    type_safe::object_ref<const cppast::cpp_type> &&ref)
{
    if (!has_type_alias_template(full_name)) {
        LOG_DBG("Stored type alias template for: {} ", full_name);

        alias_template_index_.emplace(full_name, std::move(ref));
    }
}

type_safe::object_ref<const cppast::cpp_type>
translation_unit_context::get_type_alias_template(
    const std::string &full_name) const
{
    assert(has_type_alias_template(full_name));

    return alias_template_index_.at(full_name);
}

void translation_unit_context::push_namespace(const std::string &ns)
{
    ns_ |= ns;
}

void translation_unit_context::pop_namespace() { ns_.pop_back(); }

const common::model::namespace_ &translation_unit_context::get_namespace() const
{
    return ns_;
}

const cppast::cpp_entity_index &translation_unit_context::entity_index() const
{
    return entity_index_;
}

const clanguml::config::class_diagram &translation_unit_context::config() const
{
    return config_;
}

clanguml::class_diagram::model::diagram &translation_unit_context::diagram()
{
    return diagram_;
}

clanguml::class_diagram::model::diagram &translation_unit_context::diagram() const
{
    return diagram_;
}

void translation_unit_context::set_current_package(
    type_safe::optional_ref<common::model::package> p)
{
    current_package_ = p;
}

type_safe::optional_ref<common::model::package>
translation_unit_context::get_current_package() const
{
    return current_package_;
}

void translation_unit_context::add_using_namespace_directive(
    common::model::namespace_ ns)
{
    using_ns_declarations_[ns_.to_string()].insert(std::move(ns));
}

const std::set<common::model::namespace_> &
translation_unit_context::using_namespace_directive(
    const common::model::namespace_ &ns) const
{
    return using_ns_declarations_.at(ns.to_string());
}

type_safe::optional<common::model::namespace_>
translation_unit_context::get_name_with_namespace(const std::string &name) const
{
    using common::model::namespace_;

    std::set<namespace_> possible_matches;
    possible_matches.emplace(name);

    possible_matches.emplace(get_namespace() | namespace_{name});
    auto parent = get_namespace().parent();
    while (parent.has_value()) {
        possible_matches.emplace(parent.value() | namespace_{name});
        parent = parent.value().parent();
    }

    if (using_ns_declarations_.find(get_namespace().to_string()) !=
        using_ns_declarations_.end()) {
        for (const auto &ns :
            using_ns_declarations_.at(get_namespace().to_string())) {
            possible_matches.emplace(ns | namespace_{name});
            auto parent = ns.parent();
            while (parent.has_value()) {
                possible_matches.emplace(parent.value() | namespace_{name});
                parent = parent.value().parent();
            }
        }
    }

    // Search classes
    for (const auto &c : diagram_.classes()) {
        auto c_ns = namespace_{c->name_and_ns()};
        for (const auto &possible_match : possible_matches) {
            if (c_ns == possible_match) {
                return possible_match;
            }
        }
    }

    // Search enums
    for (const auto &e : diagram_.enums()) {
        auto e_ns = namespace_{e->name_and_ns()};
        for (const auto &possible_match : possible_matches) {
            if (e_ns == possible_match) {
                return possible_match;
            }
            // Try to also match possible references to enum values
            else if (possible_match.starts_with(e_ns)) {
                return possible_match;
            }
        }
    }

    return {};
}

}
