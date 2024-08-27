/**
 * @file rc/class_diagram/generators/json/class_diagram_generator.cc
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

#include "class_diagram_generator.h"

#include "util/error.h"

namespace clanguml::class_diagram::model {
using nlohmann::json;

void set_module(nlohmann::json &j, const common::model::element &e)
{
    if (const auto &maybe_module = e.module(); maybe_module) {
        j["module"]["name"] = *maybe_module;
        j["module"]["is_private"] = e.module_private();
    }
}

void to_json(nlohmann::json &j, const class_element &c)
{
    j["name"] = c.name();
    j["type"] = common::generators::json::render_name(c.type());
    if (c.access() != common::model::access_t::kNone)
        j["access"] = to_string(c.access());
    if (!c.file().empty())
        j["source_location"] =
            dynamic_cast<const common::model::source_location &>(c);
    if (const auto &comment = c.comment(); comment)
        j["comment"] = comment.value();
}

void to_json(nlohmann::json &j, const class_member &c)
{
    j = dynamic_cast<const class_element &>(c);
    j["is_static"] = c.is_static();
}

void to_json(nlohmann::json &j, const method_parameter &c)
{
    j["name"] = c.name();
    j["type"] = c.type();
    if (!c.default_value().empty())
        j["default_value"] = c.default_value();
}

void to_json(nlohmann::json &j, const class_method &c)
{
    j = dynamic_cast<const class_element &>(c);

    j["is_pure_virtual"] = c.is_pure_virtual();
    j["is_virtual"] = c.is_virtual();
    j["is_const"] = c.is_const();
    j["is_defaulted"] = c.is_defaulted();
    j["is_deleted"] = c.is_deleted();
    j["is_static"] = c.is_static();
    j["is_noexcept"] = c.is_noexcept();
    j["is_constexpr"] = c.is_constexpr();
    j["is_consteval"] = c.is_consteval();
    j["is_coroutine"] = c.is_coroutine();
    j["is_constructor"] = c.is_constructor();
    j["is_move_assignment"] = c.is_move_assignment();
    j["is_copy_assignment"] = c.is_copy_assignment();
    j["is_operator"] = c.is_operator();
    j["template_parameters"] = c.template_params();
    j["display_name"] = c.display_name();

    j["parameters"] = c.parameters();
}

void to_json(nlohmann::json &j, const objc_method &c)
{
    j = dynamic_cast<const class_element &>(c);

    j["is_optional"] = c.is_optional();
    j["display_name"] = c.display_name();

    j["parameters"] = c.parameters();
}

void to_json(nlohmann::json &j, const class_ &c)
{
    j = dynamic_cast<const common::model::element &>(c);
    j["is_struct"] = c.is_struct();
    j["is_abstract"] = c.is_abstract();
    j["is_union"] = c.is_union();
    j["is_nested"] = c.is_nested();
    j["is_template"] = c.is_template();

    j["members"] = c.members();
    j["methods"] = c.methods();
    auto bases = nlohmann::json::array();

    for (const auto &rel : c.relationships()) {
        if (rel.type() != common::model::relationship_t::kExtension)
            continue;

        auto base = nlohmann::json::object();
        base["is_virtual"] = rel.is_virtual();
        base["id"] = std::to_string(rel.destination().value());
        if (rel.access() != common::model::access_t::kNone)
            base["access"] = to_string(rel.access());
        bases.push_back(std::move(base));
    }
    j["bases"] = std::move(bases);

    set_module(j, c);

    j["template_parameters"] = c.template_params();
}

void to_json(nlohmann::json &j, const objc_interface &c)
{
    j = dynamic_cast<const common::model::element &>(c);
    j["is_protocol"] = c.is_protocol();
    j["is_category"] = c.is_category();

    j["members"] = c.members();
    j["methods"] = c.methods();
    auto bases = nlohmann::json::array();
    auto protocols = nlohmann::json::array();

    for (const auto &rel : c.relationships()) {
        if (rel.type() == common::model::relationship_t::kExtension) {
            auto base = nlohmann::json::object();
            base["id"] = std::to_string(rel.destination().value());
            if (rel.access() != common::model::access_t::kNone)
                base["access"] = to_string(rel.access());
            bases.push_back(std::move(base));
        }
        else if (rel.type() == common::model::relationship_t::kInstantiation) {
            auto protocol = nlohmann::json::object();
            protocol["id"] = std::to_string(rel.destination().value());
            protocols.push_back(std::move(protocol));
        }
    }
    j["bases"] = std::move(bases);
    j["protocols"] = std::move(protocols);
}

void to_json(nlohmann::json &j, const enum_ &c)
{
    j = dynamic_cast<const common::model::element &>(c);
    j["is_nested"] = c.is_nested();
    j["constants"] = c.constants();

    set_module(j, c);
}

void to_json(nlohmann::json &j, const concept_ &c)
{
    j = dynamic_cast<const common::model::element &>(c);
    j["parameters"] = c.requires_parameters();
    j["statements"] = c.requires_statements();

    set_module(j, c);
}

} // namespace clanguml::class_diagram::model

namespace clanguml::class_diagram::generators::json {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_diagram(nlohmann::json &parent) const
{
    if (config().using_namespace)
        parent["using_namespace"] = config().using_namespace().to_string();

    if (config().using_module)
        parent["using_module"] = config().using_module();

    if (config().generate_packages.has_value)
        parent["package_type"] = to_string(config().package_type());
    parent["elements"] = std::vector<nlohmann::json>{};
    parent["relationships"] = std::vector<nlohmann::json>{};

    generate_top_level_elements(parent);

    generate_relationships(parent);
}

void generator::generate_top_level_elements(nlohmann::json &parent) const
{
    for (const auto &p : model()) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            if (!pkg->is_empty())
                generate(*pkg, parent);
        }
        else {
            model().dynamic_apply(
                p.get(), [&](auto *el) { generate(*el, parent); });
        }
    }
}

void generator::generate(const package &p, nlohmann::json &parent) const
{
    const auto &uns = config().using_namespace();

    nlohmann::json package_object;

    if (config().generate_packages()) {
        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            LOG_DBG("Generating package {}", p.name());

            package_object["type"] = to_string(config().package_type());
            package_object["name"] = p.name();
            package_object["display_name"] = p.name();
        }
    }

    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty()) {
                if (config().generate_packages())
                    generate(sp, package_object);
                else
                    generate(sp, parent);
            }
        }
        else {
            model().dynamic_apply(subpackage.get(), [&](auto *el) {
                if (config().generate_packages())
                    generate(*el, package_object);
                else
                    generate(*el, parent);
            });
        }
    }

    if (config().generate_packages() && !package_object.empty()) {
        parent["elements"].push_back(std::move(package_object));
    }
}

void generator::generate(const class_ &c, nlohmann::json &parent) const
{
    nlohmann::json object = c;

    // Perform config dependent postprocessing on generated class
    if (!config().generate_fully_qualified_name())
        object["display_name"] =
            common::generators::json::render_name(c.full_name_no_ns());

    object["display_name"] =
        config().simplify_template_type(object["display_name"]);

    for (auto &tp : object["template_parameters"]) {
        if (tp.contains("type") && tp.at("type").is_string()) {
            tp["type"] = config().using_namespace().relative(tp.at("type"));
        }
    }
    for (auto &tp : object["members"]) {
        if (tp.contains("type") && tp.at("type").is_string()) {
            tp["type"] = config().using_namespace().relative(tp.at("type"));
        }
    }

    parent["elements"].push_back(std::move(object));
}

void generator::generate(const enum_ &e, nlohmann::json &parent) const
{
    nlohmann::json object = e;

    if (!config().generate_fully_qualified_name())
        object["display_name"] =
            common::generators::json::render_name(e.full_name_no_ns());

    parent["elements"].push_back(std::move(object));
}

void generator::generate(const concept_ &c, nlohmann::json &parent) const
{
    nlohmann::json object = c;

    if (!config().generate_fully_qualified_name())
        object["display_name"] =
            common::generators::json::render_name(c.full_name_no_ns());

    parent["elements"].push_back(std::move(object));
}

void generator::generate(const objc_interface &c, nlohmann::json &parent) const
{
    nlohmann::json object = c;

    // Perform config dependent postprocessing on generated class
    if (!config().generate_fully_qualified_name())
        object["display_name"] =
            common::generators::json::render_name(c.full_name_no_ns());

    object["display_name"] =
        config().simplify_template_type(object["display_name"]);

    parent["elements"].push_back(std::move(object));
}

void generator::generate_relationships(nlohmann::json &parent) const
{
    for (const auto &p : model()) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            generate_relationships(*pkg, parent);
        }
        else {
            model().dynamic_apply(p.get(),
                [&](auto *el) { generate_relationships(*el, parent); });
        }
    }
}

template <>
void generator::generate_relationships<package>(
    const package &p, nlohmann::json &parent) const
{
    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty())
                generate_relationships(sp, parent);
        }
        else {
            model().dynamic_apply(subpackage.get(), [&](auto *el) {
                if (model().should_include(*el)) {
                    generate_relationships(*el, parent);
                }
            });
        }
    }
}

} // namespace clanguml::class_diagram::generators::json
