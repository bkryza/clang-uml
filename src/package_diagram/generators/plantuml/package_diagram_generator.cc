/**
 * @file src/package_diagram/generators/plantuml/package_diagram_generator.cc
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

#include "package_diagram_generator.h"

#include "util/error.h"

namespace clanguml::package_diagram::generators::plantuml {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
    , together_group_stack_{false}
{
}

void generator::generate_relationships(
    const package &p, std::ostream &ostr) const
{
    LOG_DBG("Generating relationships for package {}", p.full_name(true));

    // Generate this packages relationship
    if (model().should_include(relationship_t::kDependency)) {
        for (const auto &r : p.relationships()) {
            std::stringstream relstr;
            try {
                auto destination_package = model().get(r.destination());

                if (!destination_package ||
                    !model().should_include(
                        dynamic_cast<const package &>(*destination_package)))
                    continue;

                auto destination_alias = model().to_alias(r.destination());

                if (!destination_alias.empty()) {
                    relstr << p.alias() << " ..> " << destination_alias << '\n';
                    ostr << relstr.str();
                }
            }
            catch (error::uml_alias_missing &e) {
                LOG_DBG("=== Skipping dependency relation from {} to {} due "
                        "to: {}",
                    p.full_name(false), r.destination(), e.what());
            }
        }
    }

    // Process it's subpackages relationships
    for (const auto &subpackage : p) {
        if (model().should_include(dynamic_cast<const package &>(*subpackage)))
            generate_relationships(
                dynamic_cast<const package &>(*subpackage), ostr);
    }
}

void generator::generate(const package &p, std::ostream &ostr) const
{
    LOG_DBG("Generating package {}", p.name());

    together_group_stack_.enter();

    const auto &uns = config().using_namespace();

    // Don't generate packages from namespaces filtered out by
    // using_namespace
    if (!uns.starts_with({p.full_name(false)})) {
        ostr << "package [" << p.name() << "] ";
        ostr << "as " << p.alias();

        if (p.is_deprecated())
            ostr << " <<deprecated>>";

        if (config().generate_links) {
            generate_link(ostr, p);
        }

        generate_style(ostr, p.type_name(), p);

        ostr << " {" << '\n';
    }

    for (const auto &subpackage : p) {
        auto &pkg = dynamic_cast<package &>(*subpackage);
        if (model().should_include(pkg)) {
            auto together_group =
                config().get_together_group(pkg.full_name(false));
            if (together_group) {
                together_group_stack_.group_together(
                    together_group.value(), &pkg);
            }
            else {
                generate(pkg, ostr);
            }
        }
    }

    generate_groups(ostr);

    if (!uns.starts_with({p.full_name(false)})) {
        ostr << "}" << '\n';
    }

    generate_notes(ostr, p);

    together_group_stack_.leave();
}

void generator::generate_groups(std::ostream &ostr) const
{
    for (const auto &[group_name, group_elements] :
        together_group_stack_.get_current_groups()) {
        ostr << "together {\n";

        for (auto *pkg : group_elements) {
            generate(*pkg, ostr);
        }

        ostr << "}\n";
    }
}

void generator::generate_diagram(std::ostream &ostr) const
{
    for (const auto &p : model()) {
        auto &pkg = dynamic_cast<package &>(*p);
        if (model().should_include(pkg)) {
            auto together_group =
                config().get_together_group(pkg.full_name(false));
            if (together_group) {
                together_group_stack_.group_together(
                    together_group.value(), &pkg);
            }
            else {
                generate(pkg, ostr);
            }
        }
    }

    generate_groups(ostr);

    // Process package relationships
    for (const auto &p : model()) {
        if (model().should_include(dynamic_cast<package &>(*p)))
            generate_relationships(dynamic_cast<package &>(*p), ostr);
    }

    generate_config_layout_hints(ostr);
}

} // namespace clanguml::package_diagram::generators::plantuml
