/**
 * src/class_diagram/generators/plantuml/class_diagram_generator.cc
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

#include <inja/inja.hpp>

namespace clanguml::class_diagram::generators::plantuml {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
    , together_group_stack_{!config.generate_packages()}
{
}

void generator::generate_link(
    std::ostream &ostr, const class_diagram::model::class_element &e) const
{
    if (e.file().empty())
        return;

    auto context = element_context(e);

    if (!m_config.generate_links().link.empty()) {
        ostr << " [[[";
        ostr << env().render(
            std::string_view{m_config.generate_links().link}, context);
    }

    if (!m_config.generate_links().tooltip.empty()) {
        ostr << "{";
        ostr << env().render(
            std::string_view{m_config.generate_links().tooltip}, context);
        ostr << "}";
    }
    ostr << "]]]";
}

std::string generator::render_name(std::string name) const
{
    util::replace_all(name, "##", "::");

    return name;
}

void generator::generate_alias(const class_ &c, std::ostream &ostr) const
{
    std::string class_type{"class"};
    if (c.is_abstract())
        class_type = "abstract";

    std::string full_name;
    if (m_config.generate_packages())
        full_name = c.full_name_no_ns();
    else
        full_name = c.full_name();

    assert(!full_name.empty());

    print_debug(c, ostr);

    ostr << class_type << " \""
         << m_config.simplify_template_type(render_name(full_name));

    ostr << "\" as " << c.alias() << '\n';

    // Register the added alias
    m_generated_aliases.emplace(c.alias());
}

void generator::generate_alias(const enum_ &e, std::ostream &ostr) const
{
    print_debug(e, ostr);

    if (m_config.generate_packages())
        ostr << "enum"
             << " \"" << e.name();
    else
        ostr << "enum"
             << " \"" << render_name(e.full_name());

    ostr << "\" as " << e.alias() << '\n';

    // Register the added alias
    m_generated_aliases.emplace(e.alias());
}

void generator::generate_alias(const concept_ &c, std::ostream &ostr) const
{
    print_debug(c, ostr);

    if (m_config.generate_packages())
        ostr << "class"
             << " \"" << c.name();
    else
        ostr << "class"
             << " \"" << render_name(c.full_name());

    ostr << "\" as " << c.alias() << '\n';

    // Register the added alias
    m_generated_aliases.emplace(c.alias());
}

void generator::generate(const class_ &c, std::ostream &ostr) const
{
    namespace plantuml_common = clanguml::common::generators::plantuml;

    constexpr auto kAbbreviatedMethodArgumentsLength{15};

    const auto &uns = m_config.using_namespace();

    std::string class_type{"class"};
    if (c.is_abstract())
        class_type = "abstract";

    ostr << class_type << " " << c.alias();

    if (c.is_union())
        ostr << " "
             << "<<union>>";

    if (m_config.generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, c);
    }

    if (!c.style().empty())
        ostr << " " << c.style();

    ostr << " {" << '\n';

    //
    // Process methods
    //
    for (const auto &m : c.methods()) {
        if (!m_model.should_include(m.access()))
            continue;

        print_debug(m, ostr);

        if (m.is_pure_virtual())
            ostr << "{abstract} ";

        if (m.is_static())
            ostr << "{static} ";

        std::string type{
            uns.relative(m_config.simplify_template_type(m.type()))};

        ostr << plantuml_common::to_plantuml(m.access()) << m.name();

        if (!m.template_params().empty()) {
            m.render_template_params(ostr, m_config.using_namespace(), false);
        }

        ostr << "(";
        if (m_config.generate_method_arguments() !=
            config::method_arguments::none) {
            std::vector<std::string> params;
            std::transform(m.parameters().cbegin(), m.parameters().cend(),
                std::back_inserter(params), [this](const auto &mp) {
                    return m_config.simplify_template_type(
                        mp.to_string(m_config.using_namespace()));
                });
            auto args_string = fmt::format("{}", fmt::join(params, ", "));
            if (m_config.generate_method_arguments() ==
                config::method_arguments::abbreviated) {
                args_string = clanguml::util::abbreviate(
                    args_string, kAbbreviatedMethodArgumentsLength);
            }
            ostr << args_string;
        }
        ostr << ")";

        if (m.is_const())
            ostr << " const";

        assert(!(m.is_pure_virtual() && m.is_defaulted()));

        if (m.is_pure_virtual())
            ostr << " = 0";

        if (m.is_defaulted())
            ostr << " = default";

        ostr << " : " << type;

        if (m_config.generate_links) {
            generate_link(ostr, m);
        }

        ostr << '\n';
    }

    //
    // Process relationships - here only generate the set of
    // rendered_relationships we'll generate them in a seperate method
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;
    for (const auto &r : c.relationships()) {
        if (!m_model.should_include(r.type()))
            continue;

        LOG_DBG("Processing relationship {}",
            plantuml_common::to_plantuml(r.type(), r.style()));

        std::string destination;
        try {
            auto target_element = m_model.get(r.destination());
            if (!target_element.has_value())
                throw error::uml_alias_missing{
                    fmt::format("Missing element in the model for ID: {}",
                        r.destination())};

            destination = target_element.value().full_name(false);

            if (util::starts_with(destination, std::string{"::"}))
                destination = destination.substr(2, destination.size());

            std::string puml_relation;
            if (!r.multiplicity_source().empty())
                puml_relation += "\"" + r.multiplicity_source() + "\" ";

            puml_relation += plantuml_common::to_plantuml(r.type(), r.style());

            if (!r.multiplicity_destination().empty())
                puml_relation += " \"" + r.multiplicity_destination() + "\"";

            if (!r.label().empty()) {
                rendered_relations.emplace(r.label());
            }
        }
        catch (error::uml_alias_missing &e) {
            LOG_DBG("Skipping {} relation from {} to {} due "
                    "to: {}",
                plantuml_common::to_plantuml(r.type(), r.style()),
                c.full_name(), destination, e.what());
        }
    }

    //
    // Process members
    //
    for (const auto &m : c.members()) {
        if (!m_model.should_include(m.access()))
            continue;

        if (!m_config.include_relations_also_as_members() &&
            rendered_relations.find(m.name()) != rendered_relations.end())
            continue;

        print_debug(m, ostr);

        if (m.is_static())
            ostr << "{static} ";

        ostr << plantuml_common::to_plantuml(m.access()) << m.name() << " : "
             << render_name(
                    uns.relative(m_config.simplify_template_type(m.type())));

        if (m_config.generate_links) {
            generate_link(ostr, m);
        }

        ostr << '\n';
    }

    ostr << "}" << '\n';

    generate_notes(ostr, c);

    for (const auto &member : c.members())
        generate_member_notes(ostr, member, c.alias());

    for (const auto &method : c.methods())
        generate_member_notes(ostr, method, c.alias());
}

void generator::generate(const concept_ &c, std::ostream &ostr) const
{
    std::string class_type{"class"};

    ostr << class_type << " " << c.alias() << " <<concept>>";

    if (m_config.generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, c);
    }

    if (!c.style().empty())
        ostr << " " << c.style();

    ostr << " {" << '\n';

    // TODO: add option to enable/disable this
    if (c.requires_parameters().size() + c.requires_statements().size() > 0) {
        std::vector<std::string> parameters;
        parameters.reserve(c.requires_parameters().size());
        for (const auto &p : c.requires_parameters()) {
            parameters.emplace_back(p.to_string(m_config.using_namespace()));
        }

        ostr << fmt::format("({})\n", fmt::join(parameters, ","));

        ostr << "..\n";

        ostr << fmt::format("{}\n", fmt::join(c.requires_statements(), "\n"));
    }

    ostr << "}" << '\n';
}

void generator::generate_member_notes(std::ostream &ostr,
    const class_element &member, const std::string &alias) const
{
    for (const auto &decorator : member.decorators()) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(m_config.name)) {
            ostr << "note " << note->position << " of " << alias
                 << "::" << member.name() << '\n'
                 << note->text << '\n'
                 << "end note\n";
        }
    }
}

void generator::generate_relationships(std::ostream &ostr) const
{
    for (const auto &p : m_model) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            generate_relationships(*pkg, ostr);
        }
        else if (auto *cls = dynamic_cast<class_ *>(p.get()); cls) {
            if (m_model.should_include(*cls)) {
                generate_relationships(*cls, ostr);
            }
        }
        else if (auto *enm = dynamic_cast<enum_ *>(p.get()); enm) {
            if (m_model.should_include(*enm)) {
                generate_relationships(*enm, ostr);
            }
        }
        else if (auto *cpt = dynamic_cast<concept_ *>(p.get()); cpt) {
            if (m_model.should_include(*cpt)) {
                generate_relationships(*cpt, ostr);
            }
        }
    }
}

void generator::generate_relationships(
    const class_ &c, std::ostream &ostr) const
{
    namespace plantuml_common = clanguml::common::generators::plantuml;

    //
    // Process relationships
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;

    for (const auto &r : c.relationships()) {
        if (!m_model.should_include(r.type()))
            continue;

        LOG_DBG("== Processing relationship {}",
            plantuml_common::to_plantuml(r.type(), r.style()));

        std::stringstream relstr;
        clanguml::common::id_t destination{0};
        try {
            destination = r.destination();

            std::string puml_relation;
            if (!r.multiplicity_source().empty())
                puml_relation += "\"" + r.multiplicity_source() + "\" ";

            puml_relation += plantuml_common::to_plantuml(r.type(), r.style());

            if (!r.multiplicity_destination().empty())
                puml_relation += " \"" + r.multiplicity_destination() + "\"";

            std::string target_alias;
            try {
                target_alias = m_model.to_alias(destination);
            }
            catch (...) {
                LOG_DBG("Failed to find alias to {}", destination);
                continue;
            }

            if (m_generated_aliases.find(target_alias) ==
                m_generated_aliases.end())
                continue;

            relstr << c.alias() << " " << puml_relation << " " << target_alias;

            if (!r.label().empty()) {
                relstr << " : " << plantuml_common::to_plantuml(r.access())
                       << r.label();
                rendered_relations.emplace(r.label());
            }

            if (unique_relations.count(relstr.str()) == 0) {
                unique_relations.emplace(relstr.str());

                relstr << '\n';

                LOG_DBG("=== Adding relation {}", relstr.str());

                all_relations_str << relstr.str();
            }
        }
        catch (error::uml_alias_missing &e) {
            LOG_DBG("=== Skipping {} relation from {} to {} due "
                    "to: {}",
                plantuml_common::to_plantuml(r.type(), r.style()),
                c.full_name(), destination, e.what());
        }
    }

    if (m_model.should_include(relationship_t::kExtension)) {
        for (const auto &b : c.parents()) {
            std::stringstream relstr;
            try {
                auto target_alias = m_model.to_alias(b.id());

                if (m_generated_aliases.find(target_alias) ==
                    m_generated_aliases.end())
                    continue;

                relstr << target_alias << " <|-- " << c.alias() << '\n';
                all_relations_str << relstr.str();
            }
            catch (error::uml_alias_missing &e) {
                LOG_DBG("=== Skipping inheritance relation from {} to {} due "
                        "to: {}",
                    b.name(), c.name(), e.what());
            }
        }
    }

    ostr << all_relations_str.str();
}

void generator::generate_relationships(
    const concept_ &c, std::ostream &ostr) const
{
    namespace plantuml_common = clanguml::common::generators::plantuml;

    //
    // Process relationships
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;

    for (const auto &r : c.relationships()) {
        if (!m_model.should_include(r.type()))
            continue;

        LOG_DBG("== Processing relationship {}",
            plantuml_common::to_plantuml(r.type(), r.style()));

        std::stringstream relstr;
        clanguml::common::id_t destination{0};
        try {
            destination = r.destination();

            std::string puml_relation;
            if (!r.multiplicity_source().empty())
                puml_relation += "\"" + r.multiplicity_source() + "\" ";

            puml_relation += plantuml_common::to_plantuml(r.type(), r.style());

            if (!r.multiplicity_destination().empty())
                puml_relation += " \"" + r.multiplicity_destination() + "\"";

            std::string target_alias;
            try {
                target_alias = m_model.to_alias(destination);
            }
            catch (...) {
                LOG_DBG("Failed to find alias to {}", destination);
                continue;
            }

            if (m_generated_aliases.find(target_alias) ==
                m_generated_aliases.end())
                continue;

            relstr << c.alias() << " " << puml_relation << " " << target_alias;

            if (!r.label().empty()) {
                relstr << " : " << plantuml_common::to_plantuml(r.access())
                       << r.label();
                rendered_relations.emplace(r.label());
            }

            if (unique_relations.count(relstr.str()) == 0) {
                unique_relations.emplace(relstr.str());

                relstr << '\n';

                LOG_DBG("=== Adding relation {}", relstr.str());

                all_relations_str << relstr.str();
            }
        }
        catch (error::uml_alias_missing &e) {
            LOG_DBG("=== Skipping {} relation from {} to {} due "
                    "to: {}",
                plantuml_common::to_plantuml(r.type(), r.style()),
                c.full_name(), destination, e.what());
        }
    }

    ostr << all_relations_str.str();
}

void generator::generate(const enum_ &e, std::ostream &ostr) const
{
    ostr << "enum " << e.alias();

    if (m_config.generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, e);
    }

    if (!e.style().empty())
        ostr << " " << e.style();

    ostr << " {" << '\n';

    for (const auto &enum_constant : e.constants()) {
        ostr << enum_constant << '\n';
    }

    ostr << "}" << '\n';

    generate_notes(ostr, e);
}

void generator::generate_relationships(const enum_ &e, std::ostream &ostr) const
{
    for (const auto &r : e.relationships()) {
        if (!m_model.should_include(r.type()))
            continue;

        clanguml::common::id_t destination{0};
        std::stringstream relstr;
        try {
            destination = r.destination();

            auto target_alias = m_model.to_alias(destination);

            if (m_generated_aliases.find(target_alias) ==
                m_generated_aliases.end())
                continue;

            relstr << e.alias() << " "
                   << clanguml::common::generators::plantuml::to_plantuml(
                          r.type(), r.style())
                   << " " << target_alias;

            if (!r.label().empty())
                relstr << " : " << r.label();

            relstr << '\n';

            ostr << relstr.str();
        }
        catch (error::uml_alias_missing &ex) {
            LOG_DBG("Skipping {} relation from {} to {} due "
                    "to: {}",
                clanguml::common::generators::plantuml::to_plantuml(
                    r.type(), r.style()),
                e.full_name(), destination, ex.what());
        }
    }
}

void generator::generate(const package &p, std::ostream &ostr) const
{
    const auto &uns = m_config.using_namespace();

    if (m_config.generate_packages()) {
        LOG_DBG("Generating package {}", p.name());

        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            print_debug(p, ostr);
            ostr << "package [" << p.name() << "] ";
            ostr << "as " << p.alias();

            if (p.is_deprecated())
                ostr << " <<deprecated>>";

            if (!p.style().empty())
                ostr << " " << p.style();

            ostr << " {" << '\n';
        }
    }

    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            // TODO: add option - generate_empty_packages
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty()) {
                together_group_stack_.enter();

                generate(sp, ostr);

                together_group_stack_.leave();
            }
        }
        else if (auto *cls = dynamic_cast<class_ *>(subpackage.get()); cls) {
            if (m_model.should_include(*subpackage)) {
                auto together_group =
                    m_config.get_together_group(cls->full_name(false));
                if (together_group) {
                    together_group_stack_.group_together(
                        together_group.value(), cls);
                }
                else {
                    generate_alias(*cls, ostr);
                    generate(*cls, ostr);
                }
            }
        }
        else if (auto *enm = dynamic_cast<enum_ *>(subpackage.get()); enm) {
            if (m_model.should_include(*subpackage)) {
                auto together_group =
                    m_config.get_together_group(subpackage->full_name(false));
                if (together_group) {
                    together_group_stack_.group_together(
                        together_group.value(), enm);
                }
                else {
                    generate_alias(*enm, ostr);
                    generate(*enm, ostr);
                }
            }
        }
        else if (auto *cpt = dynamic_cast<concept_ *>(subpackage.get()); cpt) {
            if (m_model.should_include(*subpackage)) {
                auto together_group =
                    m_config.get_together_group(cpt->full_name(false));
                if (together_group) {
                    together_group_stack_.group_together(
                        together_group.value(), cpt);
                }
                else {
                    generate_alias(*cpt, ostr);
                    generate(*cpt, ostr);
                }
            }
        }
    }

    if (m_config.generate_packages()) {
        // Now generate any diagram elements which are in together
        // groups
        for (const auto &[group_name, group_elements] :
            together_group_stack_.get_current_groups()) {
            ostr << "together {\n";

            for (auto *e : group_elements) {
                if (auto *cls = dynamic_cast<class_ *>(e); cls) {
                    generate_alias(*cls, ostr);
                    generate(*cls, ostr);
                }
                if (auto *enm = dynamic_cast<enum_ *>(e); enm) {
                    generate_alias(*enm, ostr);
                    generate(*enm, ostr);
                }
                if (auto *cpt = dynamic_cast<concept_ *>(e); cpt) {
                    generate_alias(*cpt, ostr);
                    generate(*cpt, ostr);
                }
            }

            ostr << "}\n";
        }

        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            ostr << "}" << '\n';
        }
    }

    generate_notes(ostr, p);
}

void generator::generate_relationships(
    const package &p, std::ostream &ostr) const
{
    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            // TODO: add option - generate_empty_packages
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty())
                generate_relationships(sp, ostr);
        }
        else if (dynamic_cast<class_ *>(subpackage.get()) != nullptr) {
            if (m_model.should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<class_ &>(*subpackage), ostr);
            }
        }
        else if (dynamic_cast<enum_ *>(subpackage.get()) != nullptr) {
            if (m_model.should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<enum_ &>(*subpackage), ostr);
            }
        }
        else if (dynamic_cast<concept_ *>(subpackage.get()) != nullptr) {
            if (m_model.should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<concept_ &>(*subpackage), ostr);
            }
        }
    }
}

void generator::generate(std::ostream &ostr) const
{
    update_context();

    ostr << "@startuml" << '\n';

    generate_plantuml_directives(ostr, m_config.puml().before);

    generate_top_level_elements(ostr);

    generate_groups(ostr);

    generate_relationships(ostr);

    generate_config_layout_hints(ostr);

    generate_plantuml_directives(ostr, m_config.puml().after);

    ostr << "@enduml" << '\n';
}

void generator::generate_top_level_elements(std::ostream &ostr) const
{
    for (const auto &p : m_model) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            if (!pkg->is_empty())
                generate(*pkg, ostr);
        }
        else if (auto *cls = dynamic_cast<class_ *>(p.get()); cls) {
            if (m_model.should_include(*cls)) {
                auto together_group =
                    m_config.get_together_group(cls->full_name(false));
                if (together_group) {
                    together_group_stack_.group_together(
                        together_group.value(), cls);
                }
                else {
                    generate_alias(*cls, ostr);
                    generate(*cls, ostr);
                }
            }
        }
        else if (auto *enm = dynamic_cast<enum_ *>(p.get()); enm) {
            if (m_model.should_include(*enm)) {
                auto together_group =
                    m_config.get_together_group(enm->full_name(false));
                if (together_group) {
                    together_group_stack_.group_together(
                        together_group.value(), enm);
                }
                else {
                    generate_alias(*enm, ostr);
                    generate(*enm, ostr);
                }
            }
        }
        else if (auto *cpt = dynamic_cast<concept_ *>(p.get()); cpt) {
            if (m_model.should_include(*cpt)) {
                auto together_group =
                    m_config.get_together_group(cpt->full_name(false));
                if (together_group) {
                    together_group_stack_.group_together(
                        together_group.value(), cpt);
                }
                else {
                    generate_alias(*cpt, ostr);
                    generate(*cpt, ostr);
                }
            }
        }
    }
}

void generator::generate_groups(std::ostream &ostr) const
{
    for (const auto &[group_name, group_elements] :
        together_group_stack_.get_current_groups()) {
        ostr << "together {\n";

        for (auto *e : group_elements) {
            if (auto *cls = dynamic_cast<class_ *>(e); cls) {
                generate_alias(*cls, ostr);
                generate(*cls, ostr);
            }
            if (auto *enm = dynamic_cast<enum_ *>(e); enm) {
                generate_alias(*enm, ostr);
                generate(*enm, ostr);
            }
            if (auto *cpt = dynamic_cast<concept_ *>(e); cpt) {
                generate_alias(*cpt, ostr);
                generate(*cpt, ostr);
            }
        }

        ostr << "}\n";
    }
}

} // namespace clanguml::class_diagram::generators::plantuml
