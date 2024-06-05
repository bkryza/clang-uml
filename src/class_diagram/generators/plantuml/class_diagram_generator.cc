/**
 * @file src/class_diagram/generators/plantuml/class_diagram_generator.cc
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
    if (e.file_relative().empty())
        return;

    auto context = element_context(e);

    if (!config().generate_links().link.empty()) {
        ostr << " [[[";
        ostr << env().render(
            std::string_view{config().generate_links().link}, context);
    }

    if (!config().generate_links().tooltip.empty()) {
        ostr << "{";
        ostr << env().render(
            std::string_view{config().generate_links().tooltip}, context);
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
    if (!config().generate_fully_qualified_name())
        full_name = c.full_name_no_ns();
    else
        full_name = c.full_name();

    assert(!full_name.empty());

    print_debug(c, ostr);

    ostr << class_type << " \""
         << config().simplify_template_type(render_name(full_name));

    ostr << "\" as " << c.alias() << '\n';

    // Register the added alias
    m_generated_aliases.emplace(c.alias());
}

void generator::generate_alias(const enum_ &e, std::ostream &ostr) const
{
    print_debug(e, ostr);

    if (!config().generate_fully_qualified_name())
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

    if (!config().generate_fully_qualified_name())
        ostr << "class"
             << " \"" << c.full_name_no_ns();
    else
        ostr << "class"
             << " \"" << render_name(c.full_name());

    ostr << "\" as " << c.alias() << '\n';

    // Register the added alias
    m_generated_aliases.emplace(c.alias());
}

void generator::generate(const class_ &c, std::ostream &ostr) const
{
    std::string class_type{"class"};
    if (c.is_abstract())
        class_type = "abstract";

    ostr << class_type << " " << c.alias();

    if (c.is_union())
        ostr << " "
             << "<<union>>";

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, c);
    }

    generate_style(ostr, c.type_name(), c);

    ostr << " {" << '\n';

    //
    // Process methods
    //
    if (config().group_methods()) {
        generate_methods(group_methods(c.methods()), ostr);
    }
    else {
        generate_methods(c.methods(), ostr);
    }

    //
    // Process relationships - here only generate the set of
    // rendered_relationships we'll generate them in a seperate method
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    for (const auto &r : c.relationships()) {
        if (!model().should_include(r.type()))
            continue;

        try {
            generate_relationship(r, rendered_relations);
        }
        catch (error::uml_alias_missing &e) {
            LOG_DBG("Skipping {} relation from {} to {} due "
                    "to: {}",
                to_string(r.type()), c.full_name(), r.destination(), e.what());
        }
    }

    //
    // Process members
    //
    std::vector<clanguml::class_diagram::model::class_member> members{
        c.members()};

    sort_class_elements(members);

    if (config().group_methods())
        ostr << "__\n";

    for (const auto &m : members) {
        if (!model().should_include(m))
            continue;

        if (!config().include_relations_also_as_members() &&
            rendered_relations.find(m.name()) != rendered_relations.end())
            continue;

        generate_member(m, ostr);

        ostr << '\n';
    }

    ostr << "}" << '\n';

    generate_notes(ostr, c);

    for (const auto &member : c.members())
        generate_member_notes(ostr, member, c.alias());

    for (const auto &method : c.methods())
        generate_member_notes(ostr, method, c.alias());
}

void generator::generate_methods(
    const method_groups_t &methods, std::ostream &ostr) const
{
    bool is_first_non_empty_group{true};

    for (const auto &group : method_groups_) {
        const auto &group_methods = methods.at(group);
        if (!group_methods.empty()) {
            if (!is_first_non_empty_group)
                ostr << "..\n";
            is_first_non_empty_group = false;
            generate_methods(group_methods, ostr);
        }
    }
}

void generator::generate_methods(
    const std::vector<class_method> &methods, std::ostream &ostr) const
{
    auto sorted_methods = methods;
    sort_class_elements(sorted_methods);

    for (const auto &m : sorted_methods) {
        if (!model().should_include(m))
            continue;

        generate_method(m, ostr);

        ostr << '\n';
    }
}

generator::method_groups_t generator::group_methods(
    const std::vector<class_method> &methods) const
{
    std::map<std::string, std::vector<class_method>> result;

    // First get rid of methods which don't pass the filters
    std::vector<class_method> filtered_methods;
    std::copy_if(methods.cbegin(), methods.cend(),
        std::back_inserter(filtered_methods),
        [this](auto &m) { return model().should_include(m); });

    for (const auto &g : method_groups_) {
        result[g] = {};
    }

    for (const auto &m : filtered_methods) {
        if (m.is_constructor() || m.is_destructor()) {
            result["constructors"].push_back(m);
        }
        else if (m.is_copy_assignment() || m.is_move_assignment()) {
            result["assignment"].push_back(m);
        }
        else if (m.is_operator()) {
            result["operators"].push_back(m);
        }
        else {
            result["other"].push_back(m);
        }
    }

    return result;
}

void generator::generate_method(
    const class_diagram::model::class_method &m, std::ostream &ostr) const
{
    namespace plantuml_common = clanguml::common::generators::plantuml;
    const auto &uns = config().using_namespace();

    constexpr auto kAbbreviatedMethodArgumentsLength{15};

    print_debug(m, ostr);

    if (m.is_pure_virtual())
        ostr << "{abstract} ";

    if (m.is_static())
        ostr << "{static} ";

    std::string type{uns.relative(config().simplify_template_type(m.type()))};

    ostr << plantuml_common::to_plantuml(m.access()) << m.name();

    if (!m.template_params().empty()) {
        m.render_template_params(ostr, config().using_namespace(), false);
    }

    ostr << "(";
    if (config().generate_method_arguments() !=
        config::method_arguments::none) {
        std::vector<std::string> params;
        std::transform(m.parameters().cbegin(), m.parameters().cend(),
            std::back_inserter(params), [this](const auto &mp) {
                return config().simplify_template_type(
                    mp.to_string(config().using_namespace()));
            });
        auto args_string = fmt::format("{}", fmt::join(params, ", "));
        if (config().generate_method_arguments() ==
            config::method_arguments::abbreviated) {
            args_string = clanguml::util::abbreviate(
                args_string, kAbbreviatedMethodArgumentsLength);
        }
        ostr << args_string;
    }
    ostr << ")";

    if (m.is_constexpr())
        ostr << " constexpr";
    else if (m.is_consteval())
        ostr << " consteval";

    if (m.is_const())
        ostr << " const";

    if (m.is_noexcept())
        ostr << " noexcept";

    assert(!(m.is_pure_virtual() && m.is_defaulted()));

    if (m.is_pure_virtual())
        ostr << " = 0";

    if (m.is_defaulted())
        ostr << " = default";
    else if (m.is_deleted())
        ostr << " = deleted";

    if (m.is_coroutine())
        ostr << " [coroutine]";

    ostr << " : " << type;

    if (config().generate_links) {
        generate_link(ostr, m);
    }
}

void generator::generate_member(
    const class_diagram::model::class_member &m, std::ostream &ostr) const
{
    namespace plantuml_common = clanguml::common::generators::plantuml;
    const auto &uns = config().using_namespace();

    print_debug(m, ostr);

    if (m.is_static())
        ostr << "{static} ";

    ostr << plantuml_common::to_plantuml(m.access()) << m.name() << " : "
         << render_name(
                uns.relative(config().simplify_template_type(m.type())));

    if (config().generate_links) {
        generate_link(ostr, m);
    }
}

void generator::generate(const concept_ &c, std::ostream &ostr) const
{
    std::string class_type{"class"};

    ostr << class_type << " " << c.alias() << " <<concept>>";

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, c);
    }

    generate_style(ostr, c.type_name(), c);

    ostr << " {" << '\n';

    if (config().generate_concept_requirements() &&
        (c.requires_parameters().size() + c.requires_statements().size() > 0)) {
        std::vector<std::string> parameters;
        parameters.reserve(c.requires_parameters().size());
        for (const auto &p : c.requires_parameters()) {
            parameters.emplace_back(p.to_string(config().using_namespace()));
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
        if (note && note->applies_to_diagram(config().name)) {
            ostr << "note " << note->position << " of " << alias
                 << "::" << member.name() << '\n'
                 << note->text << '\n'
                 << "end note\n";
        }
    }
}

void generator::generate_relationships(std::ostream &ostr) const
{
    for (const auto &p : model()) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            generate_relationships(*pkg, ostr);
        }
        else if (auto *cls = dynamic_cast<class_ *>(p.get()); cls) {
            if (model().should_include(*cls)) {
                generate_relationships(*cls, ostr);
            }
        }
        else if (auto *enm = dynamic_cast<enum_ *>(p.get()); enm) {
            if (model().should_include(*enm)) {
                generate_relationships(*enm, ostr);
            }
        }
        else if (auto *cpt = dynamic_cast<concept_ *>(p.get()); cpt) {
            if (model().should_include(*cpt)) {
                generate_relationships(*cpt, ostr);
            }
        }
    }
}

void generator::generate_relationship(
    const relationship &r, std::set<std::string> &rendered_relations) const
{
    namespace plantuml_common = clanguml::common::generators::plantuml;

    LOG_DBG("Processing relationship {}", to_string(r.type()));

    std::string destination;

    auto target_element = model().get(r.destination());
    if (!target_element.has_value())
        throw error::uml_alias_missing{fmt::format(
            "Missing element in the model for ID: {}", r.destination())};

    destination = target_element.value().full_name(false);

    if (util::starts_with(destination, std::string{"::"}))
        destination = destination.substr(2, destination.size());

    std::string puml_relation;
    if (!r.multiplicity_source().empty())
        puml_relation += "\"" + r.multiplicity_source() + "\" ";

    puml_relation += plantuml_common::to_plantuml(r, config());

    if (!r.multiplicity_destination().empty())
        puml_relation += " \"" + r.multiplicity_destination() + "\"";

    if (!r.label().empty()) {
        rendered_relations.emplace(r.label());
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
        if (!model().should_include(r.type()))
            continue;

        LOG_DBG("== Processing relationship {}",
            plantuml_common::to_plantuml(r, config()));

        std::stringstream relstr;
        eid_t destination{};
        try {
            destination = r.destination();

            std::string puml_relation;
            if (!r.multiplicity_source().empty())
                puml_relation += "\"" + r.multiplicity_source() + "\" ";

            puml_relation += plantuml_common::to_plantuml(r, config());

            if (!r.multiplicity_destination().empty())
                puml_relation += " \"" + r.multiplicity_destination() + "\"";

            std::string target_alias;
            try {
                target_alias = model().to_alias(destination);
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
                to_string(r.type()), c.full_name(), destination, e.what());
        }
    }

    if (model().should_include(relationship_t::kExtension)) {
        for (const auto &b : c.parents()) {
            std::stringstream relstr;
            try {
                auto target_alias = model().to_alias(b.id());

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
        if (!model().should_include(r.type()))
            continue;

        LOG_DBG("== Processing relationship {}", to_string(r.type()));

        std::stringstream relstr;
        eid_t destination{};
        try {
            destination = r.destination();

            std::string puml_relation;
            if (!r.multiplicity_source().empty())
                puml_relation += "\"" + r.multiplicity_source() + "\" ";

            puml_relation += plantuml_common::to_plantuml(r, config());

            if (!r.multiplicity_destination().empty())
                puml_relation += " \"" + r.multiplicity_destination() + "\"";

            std::string target_alias;
            try {
                target_alias = model().to_alias(destination);
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
                to_string(r.type()), c.full_name(), destination, e.what());
        }
    }

    ostr << all_relations_str.str();
}

void generator::generate(const enum_ &e, std::ostream &ostr) const
{
    ostr << "enum " << e.alias();

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, e);
    }

    generate_style(ostr, e.type_name(), e);

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
        if (!model().should_include(r.type()))
            continue;

        eid_t destination{};
        std::stringstream relstr;
        try {
            destination = r.destination();

            auto target_alias = model().to_alias(destination);

            if (m_generated_aliases.find(target_alias) ==
                m_generated_aliases.end())
                continue;

            relstr << e.alias() << " "
                   << clanguml::common::generators::plantuml::to_plantuml(
                          r, config())
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
                    r, config()),
                e.full_name(), destination, ex.what());
        }
    }
}

void generator::generate(const package &p, std::ostream &ostr) const
{
    const auto &uns = config().using_namespace();

    if (config().generate_packages()) {
        LOG_DBG("Generating package {}", p.name());

        // Don't generate packages from namespaces filtered out by
        // using_namespace
        if (!uns.starts_with({p.full_name(false)})) {
            print_debug(p, ostr);
            ostr << "package [" << p.name() << "] ";
            ostr << "as " << p.alias();

            if (p.is_deprecated())
                ostr << " <<deprecated>>";

            generate_style(ostr, p.type_name(), p);

            ostr << " {" << '\n';
        }
    }

    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            // TODO: add option - generate_empty_packages
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty() &&
                !sp.all_of([this](const common::model::element &e) {
                    return !model().should_include(e);
                })) {
                together_group_stack_.enter();

                generate(sp, ostr);

                together_group_stack_.leave();
            }
        }
        else if (auto *cls = dynamic_cast<class_ *>(subpackage.get()); cls) {
            if (model().should_include(*subpackage)) {
                auto together_group =
                    config().get_together_group(cls->full_name(false));
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
            if (model().should_include(*subpackage)) {
                auto together_group =
                    config().get_together_group(subpackage->full_name(false));
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
            if (model().should_include(*subpackage)) {
                auto together_group =
                    config().get_together_group(cpt->full_name(false));
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

    if (config().generate_packages()) {
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
            generate_notes(ostr, p);
        }
    }
}

void generator::generate_relationships(
    const package &p, std::ostream &ostr) const
{
    for (const auto &subpackage : p) {
        if (dynamic_cast<package *>(subpackage.get()) != nullptr) {
            // TODO: add option - generate_empty_packages, currently
            //       packages which do not contain anything but other
            //       packages are skipped
            const auto &sp = dynamic_cast<package &>(*subpackage);
            if (!sp.is_empty() &&
                !sp.all_of([this](const common::model::element &e) {
                    return !model().should_include(e);
                }))
                generate_relationships(sp, ostr);
        }
        else if (dynamic_cast<class_ *>(subpackage.get()) != nullptr) {
            if (model().should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<class_ &>(*subpackage), ostr);
            }
        }
        else if (dynamic_cast<enum_ *>(subpackage.get()) != nullptr) {
            if (model().should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<enum_ &>(*subpackage), ostr);
            }
        }
        else if (dynamic_cast<concept_ *>(subpackage.get()) != nullptr) {
            if (model().should_include(*subpackage)) {
                generate_relationships(
                    dynamic_cast<concept_ &>(*subpackage), ostr);
            }
        }
    }
}

void generator::generate_diagram(std::ostream &ostr) const
{
    generate_top_level_elements(ostr);

    generate_groups(ostr);

    generate_relationships(ostr);

    generate_config_layout_hints(ostr);
}

void generator::generate_top_level_elements(std::ostream &ostr) const
{
    for (const auto &p : model()) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            if (!pkg->is_empty() &&
                !pkg->all_of([this](const common::model::element &e) {
                    return !model().should_include(e);
                }))
                generate(*pkg, ostr);
        }
        else if (auto *cls = dynamic_cast<class_ *>(p.get()); cls) {
            if (model().should_include(*cls)) {
                auto together_group =
                    config().get_together_group(cls->full_name(false));
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
            if (model().should_include(*enm)) {
                auto together_group =
                    config().get_together_group(enm->full_name(false));
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
            if (model().should_include(*cpt)) {
                auto together_group =
                    config().get_together_group(cpt->full_name(false));
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
