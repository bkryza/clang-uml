/**
 * @file tests/test_case_utils/test_case_utils.cc
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

#include "class_diagram/generators/mermaid/class_diagram_generator.h"
#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/visitor/translation_unit_visitor.h"
#include "common/clang_utils.h"
#include "common/compilation_database.h"
#include "common/generators/generators.h"
#include "config/config.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "include_diagram/visitor/translation_unit_visitor.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "package_diagram/visitor/translation_unit_visitor.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "sequence_diagram/visitor/translation_unit_visitor.h"
#include "util/util.h"

#include "test_case_utils.h"

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <algorithm>
#include <complex>
#include <filesystem>
#include <string>

using namespace clanguml::util;

namespace clanguml::test {

template <> std::string package_type_name<NamespacePackage>()
{
    return "namespace";
}

template <> std::string package_type_name<ModulePackage>() { return "module"; }

template <> std::string package_type_name<DirectoryPackage>()
{
    return "directory";
}

template <> bool diagram_source_t<std::string>::contains(std::string name) const
{
    return util::contains(src, name);
}

template <>
int64_t diagram_source_t<std::string>::find(
    const std::string &pattern, int64_t offset) const
{
    std::regex pattern_regex{pattern};

    std::smatch base_match;
    auto offset_it = src.begin();
    std::advance(offset_it, offset);
    bool found =
        std::regex_search(offset_it, src.end(), base_match, pattern_regex);
    if (!found)
        return -1;

    return base_match.position(0);
}

template <>
bool diagram_source_t<std::string>::search(const std::string &pattern) const
{
    return find(pattern) > -1;
}

template <>
bool diagram_source_t<nlohmann::json>::contains(std::string name) const
{
    return false;
}

template <>
bool diagram_source_t<common::generators::graphml::graphml_t>::contains(
    std::string name) const
{
    return false;
}

template <> std::string diagram_source_t<std::string>::to_string() const
{
    return src;
}

template <> std::string diagram_source_t<nlohmann::json>::to_string() const
{
    return src.dump(2);
}

template <>
std::string
diagram_source_t<common::generators::graphml::graphml_t>::to_string() const
{
    using common::generators::graphml::graphml_t;

    std::stringstream ostr;

    src.save(ostr, " ");

    return ostr.str();
}

std::string plantuml_t::get_alias(std::string name) const
{
    return memoize(
        true, [this, name](std::string x) { return get_alias_impl(x); }, name);
}

std::string plantuml_t::get_alias_impl(std::string name) const
{
    std::vector<std::regex> patterns;

    const std::string alias_regex("([A-Z]_[0-9]+)");

    util::replace_all(name, "(", "\\(");
    util::replace_all(name, ")", "\\)");
    util::replace_all(name, " ", "\\s");
    util::replace_all(name, "*", "\\*");
    util::replace_all(name, "[", "\\[");
    util::replace_all(name, "]", "\\]");

    if (diagram_type == common::model::diagram_t::kClass) {
        patterns.push_back(
            std::regex{"class\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"abstract\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"enum\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\\[" + name + "\\]\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"protocol\\s\"" + name + "\"\\sas\\s" + alias_regex});
    }
    else if (diagram_type == common::model::diagram_t::kSequence) {
        patterns.push_back(
            std::regex{"participant\\s\"" + name + "\"\\sas\\s" + alias_regex});
    }
    else if (diagram_type == common::model::diagram_t::kPackage) {
        patterns.push_back(
            std::regex{"package\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\\[" + name + "\\]\\sas\\s" + alias_regex});
    }
    else if (diagram_type == common::model::diagram_t::kInclude) {
        patterns.push_back(
            std::regex{"file\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"folder\\s\"" + name + "\"\\sas\\s" + alias_regex});
    }

    std::smatch base_match;

    for (const auto &pattern : patterns) {
        if (std::regex_search(src, base_match, pattern) &&
            base_match.size() == 2) {
            std::ssub_match base_sub_match = base_match[1];
            std::string alias = base_sub_match.str();
            return trim(alias);
        }
    }

    return fmt::format("__INVALID__ALIAS__({})", name);
}

std::string mermaid_t::get_alias(std::string name) const
{
    return memoize(
        true, [this, name](std::string x) { return get_alias_impl(x); }, name);
}

std::string mermaid_t::get_alias_class_diagram_impl(std::string name) const
{
    std::vector<std::regex> patterns;

    const std::string alias_regex("([A-Z]_[0-9]+)");

    util::replace_all(name, "(", "&lpar;");
    util::replace_all(name, ")", "&rpar;");
    util::replace_all(name, " ", "\\s");
    util::replace_all(name, "*", "\\*");
    util::replace_all(name, "[", "\\[");
    util::replace_all(name, "]", "\\]");
    util::replace_all(name, "<", "&lt;");
    util::replace_all(name, ">", "&gt;");
    util::replace_all(name, "{", "&lbrace;");
    util::replace_all(name, "}", "&rbrace;");

    patterns.push_back(
        std::regex{"class\\s" + alias_regex + "\\[\"" + name + "\"\\]"});
    patterns.push_back(
        std::regex{"subgraph\\s" + alias_regex + "\\[" + name + "\\]"});
    patterns.push_back(
        std::regex{"\\s\\s" + alias_regex + "\\[" + name + "\\]"}); // file

    std::smatch base_match;

    for (const auto &pattern : patterns) {
        if (std::regex_search(src, base_match, pattern) &&
            base_match.size() == 2) {
            std::ssub_match base_sub_match = base_match[1];
            std::string alias = base_sub_match.str();
            return trim(alias);
        }
    }

    return fmt::format("__INVALID__ALIAS__({})", name);
}

std::string mermaid_t::get_alias_sequence_diagram_impl(std::string name) const
{
    std::vector<std::regex> patterns;

    const std::string alias_regex("([A-Z]_[0-9]+)");

    util::replace_all(name, "(", "\\(");
    util::replace_all(name, ")", "\\)");
    util::replace_all(name, " ", "\\s");
    util::replace_all(name, "*", "\\*");
    util::replace_all(name, "[", "\\[");
    util::replace_all(name, "]", "\\]");

    patterns.push_back(
        std::regex{"participant\\s" + alias_regex + "\\sas\\s" + name + "\\n"});
    patterns.push_back(std::regex{"participant\\s" + alias_regex +
        "\\sas\\s<< CUDA Kernel >><br>" + name + "\\n"});
    patterns.push_back(std::regex{"participant\\s" + alias_regex +
        "\\sas\\s<< CUDA Device >><br>" + name + "\\n"});
    patterns.push_back(std::regex{"participant\\s" + alias_regex +
        "\\sas\\s<< ObjC Interface >><br>" + name + "\\n"});

    std::smatch base_match;

    for (const auto &pattern : patterns) {
        if (std::regex_search(src, base_match, pattern) &&
            base_match.size() == 2) {
            std::ssub_match base_sub_match = base_match[1];
            std::string alias = base_sub_match.str();
            return trim(alias);
        }
    }

    return fmt::format("__INVALID__ALIAS__({})", name);
}

std::string mermaid_t::get_alias_impl(std::string name) const
{
    if (diagram_type == common::model::diagram_t::kSequence)
        return get_alias_sequence_diagram_impl(name);

    return get_alias_class_diagram_impl(name);
}

std::optional<nlohmann::json> get_element_by_id(
    const nlohmann::json &j, const std::string &id)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if (e["id"] == id)
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder") {
            auto maybe_e = get_element_by_id(e, id);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_element(
    const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if (e["display_name"] == name)
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder" ||
            e["type"] == "directory" || e["type"] == "module") {
            auto maybe_e = get_element(e, name);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_element(
    const nlohmann::json &j, const std::string &name, const std::string &type)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if ((e["display_name"] == name) && (e["type"] == type))
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder" ||
            e["type"] == "directory" || e["type"] == "module") {
            auto maybe_e = get_element(e, name);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_element(
    const json_t &src, const std::string &name)
{
    return get_element(src.src, name);
}

std::optional<nlohmann::json> get_participant(
    const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("participants"))
        return {};

    std::string using_namespace{};
    if (j.contains("using_namespace")) {
        using_namespace =
            fmt::format("{}::", j["using_namespace"].get<std::string>());
    }

    for (const nlohmann::json &e : j.at("participants")) {
        if (e["display_name"] == name ||
            e["full_name"].get<std::string>().substr(using_namespace.size()) ==
                name)
            return {e};
    }

    return {};
}

nlohmann::json::const_iterator get_relationship(const nlohmann::json &j,
    const nlohmann::json &from, const nlohmann::json &to,
    const std::string &type, const std::string &label)
{
    return std::find_if(j["relationships"].begin(), j["relationships"].end(),
        [&](const auto &it) {
            auto match = (it["source"] == from) && (it["destination"] == to) &&
                (it["type"] == type);

            if (match && label.empty())
                return true;

            if (match && (it["label"] == label))
                return true;

            return false;
        });
}

nlohmann::json::const_iterator get_relationship(const nlohmann::json &j,
    const std::string &from, const std::string &to, const std::string &type,
    const std::string &label)
{
    auto source = get_element(j, from);
    auto destination = get_element(j, to);

    if (!(source && destination))
        return j["relationships"].end();

    return get_relationship(
        j, source->at("id"), destination->at("id"), type, label);
}

std::string expand_name(const nlohmann::json &j, const std::string &name)
{
    return name;
}

QualifiedName::QualifiedName(const char *n)
    : name{n}
{
}

QualifiedName::QualifiedName(std::string_view n)
    : name{n}
{
}

QualifiedName::QualifiedName(std::string_view ns_, std::string_view n)
    : ns{ns_}
    , name{n}
{
}

QualifiedName::QualifiedName(const char *ns_, const char *n)
    : ns{ns_}
    , name{n}
{
}

QualifiedName::operator std::string() const { return str(); }

std::string QualifiedName::str(bool generate_packages) const
{
    if (ns && !generate_packages)
        return fmt::format("{}::{}", ns.value(), name);

    return name;
}

File::File(const std::string &f)
    : file{f}
{
}

namespace json_helpers {
int find_message_nested(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type, const nlohmann::json &from_p,
    const nlohmann::json &to_p, int &count, const int64_t offset,
    std::optional<int32_t> chain_index)
{
    if (!j.contains("messages") && !j.contains("message_chains"))
        return -1;

    const auto &messages = !chain_index.has_value()
        ? j["messages"]
        : j["message_chains"][chain_index.value()]["messages"];

    int res{-1};

    for (const auto &m : messages) {
        if (m.contains("branches")) {
            for (const auto &b : m["branches"]) {
                auto nested_res = find_message_nested(
                    b, from, to, msg, return_type, from_p, to_p, count, offset);

                if (nested_res >= offset)
                    return nested_res;
            }
        }
        else if (m.contains("messages")) {
            auto nested_res = find_message_nested(
                m, from, to, msg, return_type, from_p, to_p, count, offset);

            if (nested_res >= offset)
                return nested_res;
        }
        else {
            if (count >= offset &&
                (m["from"]["participant_id"] == from_p["id"]) &&
                (m["to"]["participant_id"] == to_p["id"]) &&
                (m["name"] == msg) &&
                (!return_type || m["return_type"] == *return_type))
                return count;

            count++;
        }
    }

    return res;
}

int find_message_impl(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type, int64_t offset,
    std::optional<int32_t> chain_index)
{

    auto from_p = get_participant(j, from);
    auto to_p = get_participant(j, to);

    if (!from_p)
        throw std::runtime_error(
            fmt::format("Cannot find participant {}", from));

    if (!to_p)
        throw std::runtime_error(fmt::format("Cannot find participant {}", to));

    assert(from_p->is_object());
    assert(to_p->is_object());

    // TODO: support diagrams with multiple sequences...
    int count{0};

    for (const auto &seq : j["sequences"]) {
        int64_t res{-1};

        res = find_message_nested(seq, from, to, msg, return_type, *from_p,
            *to_p, count, offset, chain_index);

        if (res >= 0)
            return res;
    }

    throw std::runtime_error(fmt::format(
        "No such message {} {} {} after offset {}", from, to, msg, offset));
}

int64_t find_message(const nlohmann::json &j, const File &from, const File &to,
    const std::string &msg, int64_t offset)
{
    return find_message_impl(j, from.file, to.file, msg, {}, offset);
}

int64_t find_message(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type, int64_t offset)
{
    return find_message_impl(
        j, expand_name(j, from), expand_name(j, to), msg, return_type, offset);
}

int64_t find_message_in_chain(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type, int64_t offset,
    uint32_t chain_index)
{
    return find_message_impl(j, expand_name(j, from), expand_name(j, to), msg,
        return_type, offset, chain_index);
}

} // namespace json_helpers

int64_t find_message_in_chain(const json_t &d, const Message &msg,
    int64_t offset, bool fail, uint32_t chain_index)
{
    if (msg.is_response) {
        // TODO: Currently response are not generated as separate messages
        //       in JSON format
        return offset;
    }

    if (msg.is_entrypoint || msg.is_exitpoint)
        return offset;

    try {
        return json_helpers::find_message_in_chain(d.src, msg.from.str(),
            msg.to.str(), msg.message, msg.return_type, offset, chain_index);
    }
    catch (std::exception &e) {
        if (!fail)
            return -1;

        std::cout << "find_message_in_chain failed with " << e.what() << "\n";

        throw e;
    }
}

std::string expand_name(const graphml_t &d, const std::string &name)
{
    return name;
}

std::string get_attr_key_id(
    const graphml_t &d, std::string const &for_, std::string const &name)
{
    auto query =
        fmt::format("//key[@for='{}' and @attr.name='{}']", for_, name);

    auto key_node = d.src.select_node(query.c_str()).node();

    auto result = key_node.attribute("id").as_string();

    return result;
}

pugi::xpath_node get_element(const graphml_t &d, const QualifiedName &cls)
{
    const auto node_name_id = get_attr_key_id(d, "node", "name");

    const auto query = fmt::format("//node[data[@key='{}' and text()='{}']]",
        node_name_id, expand_name(d, cls.str(d.generate_packages)));

    auto class_node = d.src.select_node(query.c_str());
    return class_node;
}

pugi::xml_node get_element(
    const graphml_t &d, const pugi::xml_node &parent, const QualifiedName &cls)
{
    const auto node_name_id = get_attr_key_id(d, "node", "name");

    const auto query = fmt::format("node[data[@key='{}' and text()='{}']]",
        node_name_id, expand_name(d, cls.str(d.generate_packages)));

    auto class_node = parent.select_node(query.c_str());
    if (!class_node)
        return {};

    return class_node.node();
}

pugi::xpath_node get_graph_node(
    const graphml_t &d, const pugi::xml_node &parent, const std::string &name)
{
    const auto node_name_id = get_attr_key_id(d, "node", "name");

    const auto query = fmt::format(
        "node[data[@key='{}' and text()='{}'] and graph]", node_name_id, name);

    auto graph_node = parent.child("graph").select_node(query.c_str());
    return graph_node.node();
}

pugi::xpath_node get_graph(
    const graphml_t &d, const pugi::xml_node &parent, const std::string &name)
{
    const auto node_name_id = get_attr_key_id(d, "node", "name");

    const auto query = fmt::format(
        "node[data[@key='{}' and text()='{}'] and graph]", node_name_id, name);

    auto graph_node = parent.select_node(query.c_str());
    return graph_node.node().child("graph");
}

pugi::xpath_node get_element(
    const graphml_t &d, const std::string &type, const QualifiedName &cls)
{
    const auto node_type_id = get_attr_key_id(d, "node", "type");
    const auto node_name_id = get_attr_key_id(d, "node", "name");

    const auto query = fmt::format("//node[data[@key='{}' and text()='{}'] and "
                                   "data[@key='{}' and text()='{}']]",
        node_type_id, type, node_name_id,
        expand_name(d, cls.str(d.generate_packages)));

    auto class_node = d.src.select_node(query.c_str());
    return class_node;
}

pugi::xpath_node get_element(const graphml_t &d,
    const std::vector<std::string> &types, const QualifiedName &cls)
{
    pugi::xpath_node result;

    for (const auto &t : types) {
        result = get_element(d, t, cls);
        if (result)
            return result;
    }

    return result;
}

pugi::xml_node get_nested_element(
    const graphml_t &d, const std::vector<std::string> &p)
{
    if (p.empty())
        return {};

    auto it = p.begin();

    auto subgraph = get_graph_node(d, d.src.child("graphml"), *it);

    while (true) {
        if (!subgraph)
            return {};

        it++;

        if (it == p.end())
            break;

        subgraph = get_graph_node(d, subgraph.node(), *it);
    }

    if (!subgraph)
        return {};

    return subgraph.node();
}

bool has_data(const graphml_t &d, const pugi::xml_node &node,
    const std::string &data, const std::string &value)
{
    const auto data_id = get_attr_key_id(d, node.name(), data);

    const auto query =
        fmt::format("data[@key='{}' and text()='{}']", data_id, value);

    return !!node.select_node(query.c_str());
}

pugi::xml_node get_file_node(const graphml_t &d, const pugi::xml_node &parent,
    const std::filesystem::path &path_fs)
{
    assert(path_fs.is_relative());

    std::filesystem::path path_parent_fs{path_fs.parent_path()};

    if (path_parent_fs.empty())
        return get_element(
            d, parent, QualifiedName{path_fs.filename().string()});

    std::vector<std::string> p{path_parent_fs.begin(), path_parent_fs.end()};

    auto parent_node = get_nested_element(d, p);

    if (!parent_node || !has_data(d, parent_node, "type", "folder"))
        return {};

    return get_element(d, parent_node.child("graph"),
        QualifiedName{path_fs.filename().string()});
}

pugi::xpath_node get_relationship(const graphml_t &d,
    const std::string &from_id, const std::string &to_id,
    const std::string &relationship_type, const std::string &label,
    const std::string &access)
{
    const auto relationship_type_id = get_attr_key_id(d, "edge", "type");

    std::vector<std::string> data_constraints;
    data_constraints.push_back(fmt::format("data[@key='{}' and text()='{}']",
        relationship_type_id, relationship_type));

    if (!label.empty()) {
        const auto label_id = get_attr_key_id(d, "edge", "label");

        data_constraints.push_back(
            fmt::format("data[@key='{}' and text()='{}']", label_id, label));
    }

    if (!access.empty()) {
        const auto access_id = get_attr_key_id(d, "edge", "access");

        data_constraints.push_back(
            fmt::format("data[@key='{}' and text()='{}']", access_id, access));
    }

    auto query = fmt::format("//edge[@source='{}' and @target='{}' and {}]",
        from_id, to_id, fmt::join(data_constraints, " and "));

    auto result = d.src.select_node(query.c_str());

    return result;
}

bool find_relationship(const graphml_t &d, const std::string &from_id,
    const std::string &to_id, const std::string &relationship_type)
{
    return !!get_relationship(d, from_id, to_id, relationship_type);
}
} // namespace clanguml::test