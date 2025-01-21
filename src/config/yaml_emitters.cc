/**
 * @file src/config/yaml_emitters.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include "config.h"

namespace clanguml::common {

YAML::Emitter &operator<<(YAML::Emitter &out, const string_or_regex &m)
{
    if (std::holds_alternative<std::string>(m.value())) {
        out << std::get<std::string>(m.value());
    }
    else {
        out << YAML::BeginMap;
        out << YAML::Key << "r" << YAML::Value
            << std::get<regex>(m.value()).pattern;
        out << YAML::EndMap;
    }

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const namespace_or_regex &m)
{
    if (std::holds_alternative<common::model::namespace_>(m.value())) {
        out << std::get<common::model::namespace_>(m.value());
    }
    else {
        out << YAML::BeginMap;
        out << YAML::Key << "r" << YAML::Value
            << std::get<regex>(m.value()).pattern;
        out << YAML::EndMap;
    }

    return out;
}

namespace model {
YAML::Emitter &operator<<(YAML::Emitter &out, const namespace_ &n)
{
    out << n.to_string();
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const relationship_t &r)
{
    out << to_string(r);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const access_t &a)
{
    out << to_string(a);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const module_access_t &a)
{
    out << to_string(a);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const diagram_t &d)
{
    out << to_string(d);
    return out;
}

} // namespace model
} // namespace clanguml::common

namespace clanguml::config {

YAML::Emitter &operator<<(YAML::Emitter &out, const method_type &m)
{
    out << to_string(m);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const callee_type &m)
{
    out << to_string(m);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const member_order_t &r)
{
    out << to_string(r);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const package_type_t &r)
{
    out << to_string(r);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const context_config &c)
{
    out << YAML::BeginMap;
    out << YAML::Key << "match";
    out << YAML::BeginMap;
    out << YAML::Key << "radius" << YAML::Value << c.radius;
    out << YAML::Key << "pattern" << YAML::Value << c.pattern;
    out << YAML::EndMap;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const filter &f)
{
    out << YAML::BeginMap;
    if (!f.namespaces.empty())
        out << YAML::Key << "namespaces" << YAML::Value << f.namespaces;
    if (!f.modules.empty())
        out << YAML::Key << "modules" << YAML::Value << f.modules;
    if (!f.module_access.empty())
        out << YAML::Key << "module_access" << YAML::Value << f.module_access;
    if (!f.access.empty())
        out << YAML::Key << "access" << YAML::Value << f.access;
    if (!f.context.empty())
        out << YAML::Key << "context" << YAML::Value << f.context;
    if (!f.dependants.empty())
        out << YAML::Key << "dependants" << YAML::Value << f.dependants;
    if (!f.dependencies.empty())
        out << YAML::Key << "dependencies" << YAML::Value << f.dependencies;
    if (!f.elements.empty())
        out << YAML::Key << "elements" << YAML::Value << f.elements;
    if (!f.element_types.empty())
        out << YAML::Key << "element_types" << YAML::Value << f.element_types;
    if (!f.method_types.empty())
        out << YAML::Key << "method_types" << YAML::Value << f.method_types;
    if (!f.paths.empty())
        out << YAML::Key << "paths" << YAML::Value << f.paths;
    if (!f.relationships.empty())
        out << YAML::Key << "relationships" << YAML::Value << f.relationships;
    if (!f.specializations.empty())
        out << YAML::Key << "specializations" << YAML::Value
            << f.specializations;
    if (!f.subclasses.empty())
        out << YAML::Key << "subclasses" << YAML::Value << f.subclasses;
    if (!f.parents.empty())
        out << YAML::Key << "parents" << YAML::Value << f.parents;
    if (!f.method_types.empty())
        out << YAML::Key << "callee_types" << YAML::Value << f.callee_types;
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const plantuml &p)
{
    if (is_null(p)) {
        out << YAML::Null;
        return out;
    }

    out << YAML::BeginMap;
    if (!p.before.empty())
        out << YAML::Key << "before" << YAML::Value << p.before;
    if (!p.after.empty())
        out << YAML::Key << "after" << YAML::Value << p.after;
    if (!p.cmd.empty())
        out << YAML::Key << "cmd" << YAML::Value << p.cmd;
    if (!p.style.empty())
        out << YAML::Key << "style" << YAML::Value << p.style;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const mermaid &p)
{
    if (is_null(p)) {
        out << YAML::Null;
        return out;
    }

    out << YAML::BeginMap;
    if (!p.before.empty())
        out << YAML::Key << "before" << YAML::Value << p.before;
    if (!p.after.empty())
        out << YAML::Key << "after" << YAML::Value << p.after;
    if (!p.cmd.empty())
        out << YAML::Key << "cmd" << YAML::Value << p.cmd;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const graphml &p)
{
    if (is_null(p)) {
        out << YAML::Null;
        return out;
    }

    out << YAML::BeginMap;
    out << YAML::Key << "notes" << YAML::Value << p.notes;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const method_arguments &ma)
{
    out << to_string(ma);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const generate_links_config &glc)
{
    out << YAML::BeginMap;
    out << YAML::Key << "link" << YAML::Value << glc.link;
    out << YAML::Key << "tooltip" << YAML::Value << glc.tooltip;
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const git_config &gc)
{
    out << YAML::BeginMap;
    out << YAML::Key << "branch" << YAML::Value << gc.branch;
    out << YAML::Key << "revision" << YAML::Value << gc.revision;
    out << YAML::Key << "commit" << YAML::Value << gc.commit;
    out << YAML::Key << "toplevel" << YAML::Value << gc.toplevel;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const relationship_hint_t &rh)
{
    out << YAML::BeginMap;
    out << YAML::Key << "default" << YAML::Value << rh.default_hint;
    for (const auto &[k, v] : rh.argument_hints)
        out << YAML::Key << k << YAML::Value << v;
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const hint_t &h)
{
    out << to_string(h);
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const comment_parser_t &cp)
{
    out << to_string(cp);
    return out;
}

#ifdef _MSC_VER
YAML::Emitter &operator<<(YAML::Emitter &out, const std::filesystem::path &p)
{
    out << p.string();
    return out;
}

YAML::Emitter &operator<<(
    YAML::Emitter &out, const std::vector<std::filesystem::path> &paths)
{
    out << YAML::BeginSeq;
    for (const auto &p : paths)
        out << p;
    out << YAML::EndSeq;
    return out;
}
#endif

YAML::Emitter &operator<<(YAML::Emitter &out, const layout_hint &c)
{
    out << YAML::BeginMap;

    out << YAML::Key << c.hint << YAML::Value;
    if (std::holds_alternative<std::string>(c.entity))
        out << std::get<std::string>(c.entity);
    else if (std::holds_alternative<std::vector<std::string>>(c.entity))
        out << std::get<std::vector<std::string>>(c.entity);

    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const source_location &sc)
{
    out << YAML::BeginMap;
    out << YAML::Key << to_string(sc.location_type) << YAML::Value
        << sc.location;
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glob_t &g)
{
    out << YAML::BeginMap;
    out << YAML::Key << "include" << YAML::Value << g.include;
    out << YAML::Key << "exclude" << YAML::Value << g.exclude;
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const diagram_template &dt)
{
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << dt.type;
    out << YAML::Key << "description" << YAML::Value << dt.description;
    out << YAML::Key << "template" << YAML::Literal << dt.jinja_template;
    out << YAML::EndMap;

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const inja::json &j)
{
    if (j.empty() || j.is_null()) {
        out << YAML::Null;
    }
    else if (j.is_boolean()) {
        out << j.get<bool>();
    }
    else if (j.is_number_integer()) {
        out << j.get<long long>();
    }
    else if (j.is_number_unsigned()) {
        out << j.get<unsigned long long>();
    }
    else if (j.is_number_float()) {
        out << j.get<double>();
    }
    else if (j.is_string()) {
        out << j.get<std::string>();
    }
    else if (j.is_array()) {
        out << YAML::BeginSeq;
        for (const auto &element : j) {
            out << element;
        }
        out << YAML::EndSeq;
    }
    else if (j.is_object()) {
        out << YAML::BeginMap;
        for (auto it = j.begin(); it != j.end(); ++it) {
            out << YAML::Key << it.key() << YAML::Value;
            out << it.value();
        }
        out << YAML::EndMap;
    }
    else {
        out << YAML::Null;
    }
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const config &c)
{
    out << YAML::BeginMap;

    out << c.compilation_database_dir;
    out << c.output_directory;
    out << c.query_driver;
    out << c.add_compile_flags;
    out << c.remove_compile_flags;
    out << c.diagram_templates;

    out << dynamic_cast<const inheritable_diagram_options &>(c);

    if (!c.diagrams.empty()) {
        out << YAML::Key << "diagrams";
        out << YAML::BeginMap;

        for (const auto &[k, v] : c.diagrams) {
            out << YAML::Key << k;
            if (v->type() == common::model::diagram_t::kClass) {
                out << YAML::Value << dynamic_cast<class_diagram &>(*v);
            }
            else if (v->type() == common::model::diagram_t::kSequence) {
                out << YAML::Value << dynamic_cast<sequence_diagram &>(*v);
            }
            else if (v->type() == common::model::diagram_t::kInclude) {
                out << YAML::Value << dynamic_cast<include_diagram &>(*v);
            }
            else if (v->type() == common::model::diagram_t::kPackage) {
                out << YAML::Value << dynamic_cast<package_diagram &>(*v);
            }
        }
    }

    out << YAML::EndMap;
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(
    YAML::Emitter &out, const inheritable_diagram_options &c)
{
    // Common options
    out << c.base_directory;
    out << c.comment_parser;
    out << c.debug_mode;
    out << c.exclude;
    out << c.generate_links;
    out << c.git;
    out << c.glob;
    out << c.include;
    out << c.puml;
    out << c.mermaid;
    out << c.graphml;
    out << c.relative_to;
    out << c.using_namespace;
    out << c.using_module;
    out << c.generate_metadata;
    out << c.user_data;

    if (const auto *cd = dynamic_cast<const class_diagram *>(&c);
        cd != nullptr) {
        out << cd->title;
        out << c.generate_method_arguments;
        out << c.generate_concept_requirements;
        out << c.generate_packages;
        out << c.include_relations_also_as_members;
        if (c.relationship_hints) {
            out << YAML::Key << "relationship_hints" << YAML::Value
                << c.relationship_hints();
        }

        if (c.type_aliases) {
            out << YAML::Key << "type_aliases" << YAML::Value
                << c.type_aliases();
        }
        out << c.member_order;
        out << c.package_type;
        out << c.generate_template_argument_dependencies;
        out << c.skip_redundant_dependencies;
    }
    else if (const auto *sd = dynamic_cast<const sequence_diagram *>(&c);
             sd != nullptr) {
        out << sd->title;
        out << c.combine_free_functions_into_file_participants;
        out << c.inline_lambda_messages;
        out << c.generate_condition_statements;
        out << c.generate_method_arguments;
        out << c.generate_return_types;
        out << c.generate_return_values;
        out << c.participants_order;
        out << c.generate_message_comments;
        out << c.fold_repeated_activities;
        out << c.message_comment_width;
    }
    else if (const auto *pd = dynamic_cast<const package_diagram *>(&c);
             pd != nullptr) {
        out << pd->title;
        out << c.generate_packages;
        out << c.package_type;
    }
    else if (const auto *id = dynamic_cast<const include_diagram *>(&c);
             id != nullptr) {
        out << id->title;
        out << c.generate_system_headers;
    }

    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const class_diagram &c)
{
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << c.type();
    out << c.layout;
    out << dynamic_cast<const inheritable_diagram_options &>(c);
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const sequence_diagram &c)
{
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << c.type();
    out << c.from;
    out << c.from_to;
    out << c.to;
    out << dynamic_cast<const inheritable_diagram_options &>(c);
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const include_diagram &c)
{
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << c.type();
    out << c.layout;
    out << dynamic_cast<const inheritable_diagram_options &>(c);
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const package_diagram &c)
{
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << c.type();
    out << c.layout;
    out << dynamic_cast<const inheritable_diagram_options &>(c);
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const element_filter_t &ef)
{
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << to_string(ef.type);
    out << YAML::Key << "name" << YAML::Value << ef.name;
    out << YAML::EndMap;

    return out;
}

template <> bool is_null(const std::string &v) { return v.empty(); }

template <> bool is_null(const glob_t &v)
{
    return v.include.empty() && v.exclude.empty();
}

template <> bool is_null(const plantuml &v)
{
    return v.before.empty() && v.after.empty() && v.cmd.empty() &&
        v.style.empty();
}

template <> bool is_null(const mermaid &v)
{
    return v.before.empty() && v.after.empty() && v.cmd.empty();
}

template <> bool is_null(const graphml &v) { return v.notes.empty(); }

template <> bool is_null(const inja::json &v) { return v.is_null(); }

} // namespace clanguml::config