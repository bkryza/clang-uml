/**
 * @file src/class_diagram/generators/mermaid/class_diagram_generator.cc
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

namespace clanguml::class_diagram::generators::mermaid {

using clanguml::common::eid_t;
using clanguml::common::generators::mermaid::escape_name;
using clanguml::common::generators::mermaid::indent;
using clanguml::common::generators::mermaid::render_name;

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
    , class_diagram::generators::text_diagram_strategy<generator>{*this, false}

{
}

void generator::generate_diagram_type(std::ostream &ostr) const
{
    ostr << "classDiagram\n";
}

void generator::generate_alias(
    const common::model::element &c, std::ostream &ostr) const
{
    const auto full_name = c.full_name(true);

    assert(!full_name.empty());

    print_debug(c, ostr);

    auto class_label = config().simplify_template_type(render_name(full_name));

    ostr << indent(1) << "class " << c.alias() << "[\""
         << escape_name(class_label) << "\"]\n";

    // Register the added alias
    m_generated_aliases.emplace(c.alias());
}

void generator::generate(const class_ &c, std::ostream &ostr) const
{
    std::string class_type{"class"};

    ostr << indent(1) << "class " << c.alias();

    ostr << " {" << '\n';

    if (c.is_union())
        ostr << indent(2) << "<<union>>\n";
    else if (c.is_abstract())
        ostr << indent(2) << "<<abstract>>\n";

    //
    // Process methods
    //
    if (config().group_methods()) {
        generate_method_groups(group_methods(c.methods()), ostr);
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

    for (const auto &m : members) {
        if (!config().include_relations_also_as_members() &&
            rendered_relations.find(m.name()) != rendered_relations.end())
            continue;

        generate_member(m, ostr);

        ostr << '\n';
    }

    ostr << indent(1) << "}" << '\n';

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, c);
    }

    generate_notes(ostr, c);

    for (const auto &member : c.members())
        generate_member_notes(ostr, member, c.alias());

    for (const auto &method : c.methods())
        generate_member_notes(ostr, method, c.alias());
}

void generator::start_method_group(std::ostream &ostr) const { }

void generator::generate_methods(
    const std::vector<class_method> &methods, std::ostream &ostr) const
{
    auto sorted_methods = methods;
    sort_class_elements(sorted_methods);

    for (const auto &m : sorted_methods) {
        generate_method(m, ostr);
        ostr << '\n';
    }
}

void generator::generate_methods(
    const std::vector<objc_method> &methods, std::ostream &ostr) const
{
    auto sorted_methods = methods;
    sort_class_elements(sorted_methods);

    for (const auto &m : sorted_methods) {
        generate_method(m, ostr);
        ostr << '\n';
    }
}

void generator::generate_method(
    const class_diagram::model::class_method &m, std::ostream &ostr) const
{
    namespace mermaid_common = clanguml::common::generators::mermaid;
    const auto &uns = config().using_namespace();

    constexpr auto kAbbreviatedMethodArgumentsLength{15};

    print_debug(m, ostr);

    std::string type{uns.relative(config().simplify_template_type(m.type()))};

    ostr << indent(2) << mermaid_common::to_mermaid(m.access()) << m.name();

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
        ostr << escape_name(args_string);
    }
    ostr << ")";

    ostr << " : ";

    std::vector<std::string> method_mods;
    if (m.is_defaulted()) {
        method_mods.emplace_back("default");
    }
    if (m.is_const()) {
        method_mods.emplace_back("const");
    }
    if (m.is_constexpr()) {
        method_mods.emplace_back("constexpr");
    }
    if (m.is_consteval()) {
        method_mods.emplace_back("consteval");
    }
    if (m.is_coroutine()) {
        method_mods.emplace_back("coroutine");
    }

    if (!method_mods.empty()) {
        ostr << fmt::format("[{}] ", fmt::join(method_mods, ","));
    }

    ostr << escape_name(render_name(type));

    if (m.is_pure_virtual())
        ostr << "*";

    if (m.is_static())
        ostr << "$";
}

void generator::generate_method(
    const class_diagram::model::objc_method &m, std::ostream &ostr) const
{
    namespace mermaid_common = clanguml::common::generators::mermaid;
    const auto &uns = config().using_namespace();

    constexpr auto kAbbreviatedMethodArgumentsLength{15};

    print_debug(m, ostr);

    std::string type{uns.relative(config().simplify_template_type(m.type()))};

    ostr << indent(2) << mermaid_common::to_mermaid(m.access()) << m.name();

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
        ostr << escape_name(args_string);
    }
    ostr << ")";

    ostr << " : ";

    std::vector<std::string> method_mods;
    if (m.is_optional()) {
        method_mods.emplace_back("default");
    }

    if (!method_mods.empty()) {
        ostr << fmt::format("[{}] ", fmt::join(method_mods, ","));
    }

    ostr << escape_name(render_name(type));

    if (m.is_static())
        ostr << "$";
}

void generator::generate_member(
    const class_diagram::model::class_member &m, std::ostream &ostr) const
{
    namespace mermaid_common = clanguml::common::generators::mermaid;
    const auto &uns = config().using_namespace();

    print_debug(m, ostr);

    ostr << indent(2) << mermaid_common::to_mermaid(m.access()) << m.name()
         << " : "
         << escape_name(uns.relative(
                config().simplify_template_type(render_name(m.type()))));
}

void generator::generate_member(
    const class_diagram::model::objc_member &m, std::ostream &ostr) const
{
    namespace mermaid_common = clanguml::common::generators::mermaid;
    const auto &uns = config().using_namespace();

    print_debug(m, ostr);

    ostr << indent(2) << mermaid_common::to_mermaid(m.access()) << m.name()
         << " : "
         << escape_name(uns.relative(
                config().simplify_template_type(render_name(m.type()))));
}

void generator::generate(const concept_ &c, std::ostream &ostr) const
{
    ostr << indent(1) << "class"
         << " " << c.alias();

    ostr << " {" << '\n';
    ostr << indent(2) << "<<concept>>\n";

    if (config().generate_concept_requirements() &&
        (c.requires_parameters().size() + c.requires_statements().size() > 0)) {
        std::vector<std::string> parameters;
        parameters.reserve(c.requires_parameters().size());
        for (const auto &p : c.requires_parameters()) {
            parameters.emplace_back(
                escape_name(p.to_string(config().using_namespace())));
        }

        ostr << indent(2)
             << fmt::format("\"({})\"\n", fmt::join(parameters, ","));

        for (const auto &req : c.requires_statements()) {
            ostr << indent(2)
                 << fmt::format("\"{}\"\n", escape_name(req, false));
        }
    }

    ostr << indent(1) << "}" << '\n';

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, c);
    }
}

void generator::generate_member_notes(std::ostream &ostr,
    const class_element &member, const std::string &alias) const
{
    for (const auto &decorator : member.decorators()) {
        auto note = std::dynamic_pointer_cast<decorators::note>(decorator);
        if (note && note->applies_to_diagram(config().name)) {
            ostr << indent(1) << "note for " << alias << " \"" << note->text
                 << "\"" << '\n';
        }
    }
}

void generator::generate_relationship(
    const relationship &r, std::set<std::string> &rendered_relations) const
{
    namespace mermaid_common = clanguml::common::generators::mermaid;

    LOG_DBG("Processing relationship {}", to_string(r.type()));

    std::string destination;

    auto target_element = model().get(r.destination());
    if (!target_element.has_value())
        throw error::uml_alias_missing{fmt::format(
            "Missing element in the model for ID: {}", r.destination())};

    destination = target_element.value().full_name(false);

    if (util::starts_with(destination, std::string{"::"}))
        destination = destination.substr(2, destination.size());

    std::string mmd_relation;
    if (!r.multiplicity_source().empty())
        mmd_relation += "\"" + r.multiplicity_source() + "\" ";

    mmd_relation += mermaid_common::to_mermaid(r.type());

    if (!r.multiplicity_destination().empty())
        mmd_relation += " \"" + r.multiplicity_destination() + "\"";

    if (!r.label().empty()) {
        if (r.type() == relationship_t::kFriendship)
            rendered_relations.emplace(fmt::format(
                "{}[friend]", mermaid_common::to_mermaid(r.access())));
        else
            rendered_relations.emplace(r.label());
    }
}

void generator::generate_relationships(
    const class_ &c, std::ostream &ostr) const
{
    namespace mermaid_common = clanguml::common::generators::mermaid;

    //
    // Process relationships
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;

    for (const auto &r : c.relationships()) {
        LOG_DBG("== Processing relationship {}", to_string(r.type()));

        std::stringstream relstr;
        eid_t destination{};
        try {
            destination = r.destination();

            std::string relation_str;

            if (!r.multiplicity_source().empty())
                relation_str += "\"" + r.multiplicity_source() + "\" ";

            relation_str += mermaid_common::to_mermaid(r.type());

            if (!r.multiplicity_destination().empty())
                relation_str += " \"" + r.multiplicity_destination() + "\"";

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

            if (r.type() == relationship_t::kContainment) {
                relstr << indent(1) << target_alias << " " << relation_str
                       << " " << c.alias();
            }
            else if (r.type() == relationship_t::kExtension) {
                relstr << indent(1) << target_alias << " <|-- " << c.alias();
            }
            else {
                relstr << indent(1) << c.alias() << " " << relation_str << " "
                       << target_alias;
            }

            relstr << " : ";

            if (!r.label().empty()) {
                auto lbl = r.label();
                if (r.type() == relationship_t::kFriendship)
                    lbl = "[friend]";
                relstr << mermaid_common::to_mermaid(r.access()) << lbl;
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

void generator::generate_relationships(
    const concept_ &c, std::ostream &ostr) const
{
    namespace mermaid_common = clanguml::common::generators::mermaid;

    //
    // Process relationships
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;

    for (const auto &r : c.relationships()) {
        LOG_DBG("== Processing relationship {}", to_string(r.type()));

        std::stringstream relstr;
        eid_t destination{};
        try {
            destination = r.destination();

            std::string mmd_relation;
            if (!r.multiplicity_source().empty())
                mmd_relation += "\"" + r.multiplicity_source() + "\" ";

            mmd_relation += mermaid_common::to_mermaid(r.type());

            if (!r.multiplicity_destination().empty())
                mmd_relation += " \"" + r.multiplicity_destination() + "\"";

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

            if (r.type() == relationship_t::kContainment) {
                relstr << indent(1) << target_alias << " " << mmd_relation
                       << " " << c.alias();
            }
            else {
                relstr << indent(1) << c.alias() << " " << mmd_relation << " "
                       << target_alias;
            }

            relstr << " : ";

            if (!r.label().empty()) {
                auto lbl = r.label();
                if (r.type() == relationship_t::kFriendship)
                    lbl = "[friend]";
                relstr << mermaid_common::to_mermaid(r.access()) << lbl;
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

void generator::generate_relationships(const enum_ &e, std::ostream &ostr) const
{
    for (const auto &r : e.relationships()) {
        eid_t destination{};
        std::stringstream relstr;
        try {
            destination = r.destination();

            auto target_alias = model().to_alias(destination);

            if (m_generated_aliases.find(target_alias) ==
                m_generated_aliases.end())
                continue;

            if (r.type() == relationship_t::kContainment) {
                relstr << indent(1) << target_alias << " "
                       << clanguml::common::generators::mermaid::to_mermaid(
                              r.type())
                       << " " << e.alias();
            }
            else {
                relstr << indent(1) << e.alias() << " "
                       << clanguml::common::generators::mermaid::to_mermaid(
                              r.type())
                       << " " << target_alias;
            }

            auto lbl = r.label();
            if (r.type() == relationship_t::kFriendship)
                lbl = "[friend]";

            relstr << " : " << r.label();

            relstr << '\n';

            ostr << relstr.str();
        }
        catch (error::uml_alias_missing &ex) {
            LOG_DBG("Skipping {} relation from {} to {} due "
                    "to: {}",
                clanguml::common::generators::mermaid::to_mermaid(r.type()),
                e.full_name(), destination, ex.what());
        }
    }
}

void generator::generate(const enum_ &e, std::ostream &ostr) const
{
    ostr << indent(1) << "class " << e.alias();

    ostr << " {" << '\n';

    ostr << indent(2) << "<<enumeration>>\n";

    for (const auto &enum_constant : e.constants()) {
        ostr << indent(2) << enum_constant << '\n';
    }

    ostr << indent(1) << "}" << '\n';

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, e);
    }

    generate_notes(ostr, e);
}

void generator::generate(const objc_interface &c, std::ostream &ostr) const
{
    std::string class_type{"class"};

    ostr << indent(1) << "class " << c.alias();

    ostr << " {" << '\n';

    if (c.is_protocol())
        ostr << indent(2) << "<< ObjC Protocol >>\n";
    else if (c.is_protocol())
        ostr << indent(2) << "<< ObjC Category >>\n";
    else
        ostr << indent(2) << "<< ObjC Interface >>\n";

    //
    // Process methods
    //
    generate_methods(c.methods(), ostr);

    //
    // Process relationships - here only generate the set of
    // rendered_relationships we'll generate them in a seperate method
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    for (const auto &r : c.relationships()) {
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
    std::vector<clanguml::class_diagram::model::objc_member> members{
        c.members()};

    sort_class_elements(members);

    for (const auto &m : members) {
        if (!config().include_relations_also_as_members() &&
            rendered_relations.find(m.name()) != rendered_relations.end())
            continue;

        generate_member(m, ostr);

        ostr << '\n';
    }

    ostr << indent(1) << "}" << '\n';

    if (config().generate_links) {
        common_generator<diagram_config, diagram_model>::generate_link(ostr, c);
    }

    generate_notes(ostr, c);

    for (const auto &member : c.members())
        generate_member_notes(ostr, member, c.alias());

    for (const auto &method : c.methods())
        generate_member_notes(ostr, method, c.alias());
}

void generator::generate_relationships(
    const objc_interface &c, std::ostream &ostr) const
{
    namespace mermaid_common = clanguml::common::generators::mermaid;

    //
    // Process relationships
    //
    std::set<std::string> rendered_relations;

    std::stringstream all_relations_str;
    std::set<std::string> unique_relations;

    for (const auto &r : c.relationships()) {
        LOG_DBG("== Processing relationship {}", to_string(r.type()));

        std::stringstream relstr;
        eid_t destination{};
        try {
            destination = r.destination();

            std::string relation_str;

            if (!r.multiplicity_source().empty())
                relation_str += "\"" + r.multiplicity_source() + "\" ";

            relation_str += mermaid_common::to_mermaid(r.type());

            if (!r.multiplicity_destination().empty())
                relation_str += " \"" + r.multiplicity_destination() + "\"";

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

            if (r.type() == relationship_t::kExtension) {
                relstr << indent(1) << target_alias << " <|-- " << c.alias();
            }
            else {
                relstr << indent(1) << c.alias() << " " << relation_str << " "
                       << target_alias;
            }

            relstr << " : ";

            if (!r.label().empty()) {
                auto lbl = r.label();
                if (r.type() == relationship_t::kFriendship)
                    lbl = "[friend]";
                relstr << mermaid_common::to_mermaid(r.access()) << lbl;
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

void generator::generate_diagram(std::ostream &ostr) const
{
    generate_top_level_elements(ostr);

    generate_groups(ostr);

    generate_relationships(ostr);
}

void generator::start_together_group(
    const std::string &group_name, std::ostream &ostr) const
{
}

void generator::end_together_group(
    const std::string &group_name, std::ostream &ostr) const
{
}

void generator::start_package(const package &p, std::ostream &ostr) const { }

void generator::end_package(const package &p, std::ostream &ostr) const { }

} // namespace clanguml::class_diagram::generators::mermaid
