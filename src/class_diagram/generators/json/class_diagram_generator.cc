/**
 * src/class_diagram/generators/json/class_diagram_generator.cc
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

#include "class_diagram_generator.h"

#include "util/error.h"

namespace clanguml::common::model {
using nlohmann::json;

void to_json(nlohmann::json &j, const source_location &sl)
{
    j = json{{"file", sl.file()}, {"line", sl.line()}};
}

void to_json(nlohmann::json &j, const element &c)
{
    j = json{{"id", std::to_string(c.id())}, {"name", c.name()},
        {"namespace", c.get_namespace().to_string()}, {"type", c.type_name()},
        {"display_name", c.full_name(false)}};

    if (c.comment())
        j["comment"] = c.comment().value();

    if (!c.file().empty())
        j["source_location"] =
            dynamic_cast<const common::model::source_location &>(c);
}

void to_json(nlohmann::json &j, const template_parameter &c)
{
    j["type"] = c.type();
    j["name"] = c.name();
    if (!c.default_value().empty())
        j["default_value"] = c.default_value();
    j["is_template_parameter"] = c.is_template_parameter();
    j["is_template_template_parameter"] = c.is_template_template_parameter();
    if (c.concept_constraint())
        j["concept_constraint"] = c.concept_constraint().value();
    j["is_variadic"] = c.is_variadic();
}

void to_json(nlohmann::json &j, const relationship &c)
{
    j["type"] = to_string(c.type());
    j["destination"] = std::to_string(c.destination());
    if (!c.multiplicity_source().empty())
        j["multiplicity_source"] = c.multiplicity_source();
    if (!c.multiplicity_destination().empty())
        j["multiplicity_destination"] = c.multiplicity_destination();
    if (c.access() != access_t::kNone)
        j["access"] = to_string(c.access());
    if (!c.label().empty())
        j["label"] = c.label();
    if (c.comment())
        j["comment"] = c.comment().value();
}

}

namespace clanguml::class_diagram::model {
using nlohmann::json;
void to_json(nlohmann::json &j, const class_element &c)
{
    j["name"] = c.name();
    j["type"] = c.type();
    if (c.access() != common::model::access_t::kNone)
        j["access"] = to_string(c.access());
    if (!c.file().empty())
        j["source_location"] =
            dynamic_cast<const common::model::source_location &>(c);
    if (c.comment())
        j["comment"] = c.comment().value();
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

    j["is_static"] = c.is_static();
    j["is_const"] = c.is_const();
    j["is_defaulted"] = c.is_defaulted();
    j["is_pure_virtual"] = c.is_pure_virtual();
    j["is_virtual"] = c.is_virtual();
    j["is_implicit"] = c.is_implicit();

    j["parameters"] = c.parameters();
}

void to_json(nlohmann::json &j, const class_parent &c)
{
    j["is_virtual"] = c.is_virtual();
    j["id"] = std::to_string(c.id());
    if (c.access() != common::model::access_t::kNone)
        j["access"] = to_string(c.access());
    j["name"] = c.name();
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
    j["bases"] = c.parents();

    j["template_parameters"] = c.templates();
}

void to_json(nlohmann::json &j, const enum_ &c)
{
    j = dynamic_cast<const common::model::element &>(c);
    j["is_nested"] = c.is_nested();
    j["constants"] = c.constants();
}

void to_json(nlohmann::json &j, const concept_ &c)
{
    j = dynamic_cast<const common::model::element &>(c);
    j["parameters"] = c.requires_parameters();
    j["statements"] = c.requires_statements();
}
}

namespace clanguml::class_diagram::generators::json {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate(std::ostream &ostr) const
{
    generate_top_level_elements(json_);

    generate_relationships(json_);

    ostr << json_;
}

void generator::generate_top_level_elements(nlohmann::json &parent) const
{
    for (const auto &p : m_model) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            if (!pkg->is_empty())
                generate(*pkg, parent);
        }
        else if (auto *cls = dynamic_cast<class_ *>(p.get()); cls) {
            if (m_model.should_include(*cls)) {
                generate(*cls, parent);
            }
        }
        else if (auto *enm = dynamic_cast<enum_ *>(p.get()); enm) {
            if (m_model.should_include(*enm)) {
                generate(*enm, parent);
            }
        }
        else if (auto *cpt = dynamic_cast<concept_ *>(p.get()); cpt) {
            if (m_model.should_include(*cpt)) {
                generate(*cpt, parent);
            }
        }
    }
}

void generator::generate(const package &p, nlohmann::json &parent) const
{
    const auto &uns = m_config.using_namespace();

    nlohmann::json package_object;

    if (m_config.generate_packages()) {
        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            LOG_DBG("Generating package {}", p.name());

            package_object["type"] = "namespace";
            package_object["name"] = p.name();
            package_object["display_name"] = p.full_name(false);
        }
    }

    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty()) {
                if (m_config.generate_packages())
                    generate(sp, package_object);
                else
                    generate(sp, parent);
            }
        }
        else if (auto *cls = dynamic_cast<class_ *>(subpackage.get()); cls) {
            if (m_model.should_include(*cls)) {
                if (m_config.generate_packages())
                    generate(*cls, package_object);
                else
                    generate(*cls, parent);
            }
        }
        else if (auto *enm = dynamic_cast<enum_ *>(subpackage.get()); enm) {
            if (m_model.should_include(*enm)) {
                if (m_config.generate_packages())
                    generate(*enm, package_object);
                else
                    generate(*enm, parent);
            }
        }
        else if (auto *cpt = dynamic_cast<concept_ *>(subpackage.get()); cpt) {
            if (m_model.should_include(*cpt)) {
                if (m_config.generate_packages())
                    generate(*cpt, package_object);
                else
                    generate(*cpt, parent);
            }
        }
    }

    if (m_config.generate_packages() && !package_object.empty()) {
        parent["elements"].push_back(std::move(package_object));
    }
}

void generator::generate(const class_ &c, nlohmann::json &parent) const
{
    nlohmann::json object = c;
    parent["elements"].push_back(std::move(object));
}

void generator::generate(const enum_ &e, nlohmann::json &parent) const
{
    nlohmann::json object = e;
    parent["elements"].push_back(std::move(object));
}

void generator::generate(const concept_ &c, nlohmann::json &parent) const
{
    nlohmann::json object = c;
    parent["elements"].push_back(std::move(object));
}

void generator::generate_relationships(nlohmann::json &parent) const
{
    for (const auto &p : m_model) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            generate_relationships(*pkg, parent);
        }
        else if (auto *cls = dynamic_cast<class_ *>(p.get()); cls) {
            if (m_model.should_include(*cls)) {
                generate_relationships(*cls, parent);
            }
        }
        else if (auto *enm = dynamic_cast<enum_ *>(p.get()); enm) {
            if (m_model.should_include(*enm)) {
                generate_relationships(*enm, parent);
            }
        }
        else if (auto *cpt = dynamic_cast<concept_ *>(p.get()); cpt) {
            if (m_model.should_include(*cpt)) {
                generate_relationships(*cpt, parent);
            }
        }
    }
}

void generator::generate_relationships(
    const class_ &c, nlohmann::json &parent) const
{
    for (const auto &r : c.relationships()) {
        auto target_element = m_model.get(r.destination());
        if (!target_element.has_value()) {
            LOG_DBG("Skipping {} relation from {} to {} due "
                    "to unresolved destination id",
                to_string(r.type()), c.full_name(), r.destination());
            continue;
        }

        nlohmann::json rel = r;
        rel["source"] = std::to_string(c.id());
        parent["relationships"].push_back(rel);
    }

    if (m_model.should_include(relationship_t::kExtension)) {
        for (const auto &b : c.parents()) {
            common::model::relationship r(
                relationship_t::kExtension, b.id(), b.access());
            nlohmann::json rel = r;
            rel["source"] = std::to_string(c.id());
            parent["relationships"].push_back(rel);
        }
    }
}

void generator::generate_relationships(
    const enum_ &c, nlohmann::json &parent) const
{
    for (const auto &r : c.relationships()) {
        auto target_element = m_model.get(r.destination());
        if (!target_element.has_value()) {
            LOG_DBG("Skipping {} relation from {} to {} due "
                    "to unresolved destination id",
                to_string(r.type()), c.full_name(), r.destination());
            continue;
        }

        nlohmann::json rel = r;
        rel["source"] = std::to_string(c.id());
        parent["relationships"].push_back(rel);
    }
}

void generator::generate_relationships(
    const concept_ &c, nlohmann::json &parent) const
{
    for (const auto &r : c.relationships()) {
        auto target_element = m_model.get(r.destination());
        if (!target_element.has_value()) {
            LOG_DBG("Skipping {} relation from {} to {} due "
                    "to unresolved destination id",
                to_string(r.type()), c.full_name(), r.destination());
            continue;
        }

        nlohmann::json rel = r;
        rel["source"] = std::to_string(c.id());
        parent["relationships"].push_back(rel);
    }
}

void generator::generate_relationships(
    const package &p, nlohmann::json &parent) const
{
    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty())
                generate_relationships(sp, parent);
        }
        else if (dynamic_cast<class_ *>(subpackage.get()) != nullptr) {
            if (m_model.should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<class_ &>(*subpackage), parent);
            }
        }
        else if (dynamic_cast<enum_ *>(subpackage.get()) != nullptr) {
            if (m_model.should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<enum_ &>(*subpackage), parent);
            }
        }
        else if (dynamic_cast<concept_ *>(subpackage.get()) != nullptr) {
            if (m_model.should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<concept_ &>(*subpackage), parent);
            }
        }
    }
}

} // namespace clanguml::class_diagram::generators::plantuml
