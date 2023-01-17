/**
 * src/config/yaml_emitters.cc
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

#include "config.h"

namespace clanguml::common::model {
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

YAML::Emitter &operator<<(YAML::Emitter &out, const diagram_t &d)
{
    out << to_string(d);
    return out;
}
} // namespace clanguml::common::model

namespace clanguml::config {
YAML::Emitter &operator<<(YAML::Emitter &out, const filter &f)
{
    out << YAML::BeginMap;
    if (!f.namespaces.empty())
        out << YAML::Key << "namespaces" << YAML::Value << f.namespaces;
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
    if (!f.paths.empty())
        out << YAML::Key << "paths" << YAML::Value << f.paths;
    if (!f.relationships.empty())
        out << YAML::Key << "relationships" << YAML::Value << f.relationships;
    if (!f.specializations.empty())
        out << YAML::Key << "specializations" << YAML::Value
            << f.specializations;
    if (!f.subclasses.empty())
        out << YAML::Key << "subclasses" << YAML::Value << f.subclasses;

    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const plantuml &p)
{
    if (p.before.empty() && p.after.empty())
        return out;

    out << YAML::BeginMap;
    if (!p.before.empty())
        out << YAML::Key << "before" << YAML::Value << p.before;
    if (!p.after.empty())
        out << YAML::Key << "after" << YAML::Value << p.after;
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

YAML::Emitter &operator<<(YAML::Emitter &out, const layout_hint &c)
{
    out << YAML::BeginMap;
    out << YAML::Key << c.hint << YAML::Value << c.entity;
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const source_location &sc)
{
    out << YAML::BeginMap;
    out << YAML::Key << "location" << YAML::Value << sc.location;
    out << YAML::Key << "location_type" << YAML::Value
        << to_string(sc.location_type);
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const config &c)
{
    out << YAML::BeginMap;

    out << c.compilation_database_dir;
    out << c.output_directory;

    out << dynamic_cast<const inheritable_diagram_options &>(c);

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

    out << YAML::EndMap;
    out << YAML::EndMap;
    return out;
}

YAML::Emitter &operator<<(
    YAML::Emitter &out, const inheritable_diagram_options &c)
{
    out << c.glob;
    out << c.using_namespace;
    out << c.include_relations_also_as_members;
    out << c.include;
    out << c.exclude;
    out << c.puml;
    out << c.generate_method_arguments;
    out << c.generate_packages;
    out << c.generate_links;
    out << c.git;
    out << c.base_directory;
    out << c.relative_to;
    out << c.generate_system_headers;
    if (c.relationship_hints) {
        out << YAML::Key << "relationship_hints" << YAML::Value
            << c.relationship_hints();
    }
    if (c.type_aliases) {
        out << YAML::Key << "type_aliases" << YAML::Value << c.type_aliases();
    }
    out << c.comment_parser;
    out << c.combine_free_functions_into_file_participants;
    out << c.participants_order;
    out << c.debug_mode;

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
    out << c.start_from;
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

} // namespace clanguml::config