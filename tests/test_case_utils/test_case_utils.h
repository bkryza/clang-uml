/**
 * @file tests/test_case_utils/test_case_utils.h
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
#pragma once

using namespace clanguml::util;

namespace clanguml::test {

template <typename T, typename... Ts> constexpr bool has_type() noexcept
{
    return (std::is_same_v<T, Ts> || ... || false);
}

struct Public { };

struct Protected { };

struct Private { };

struct Abstract { };

struct Static { };

struct Const { };

struct Constexpr { };

struct Consteval { };

struct Coroutine { };

struct Noexcept { };

struct Default { };

struct Deleted { };

struct Entrypoint { };

struct Exitpoint { };

struct CUDAKernel { };
struct CUDADevice { };

struct CoAwait { };
struct CoYield { };
struct CoReturn { };

struct ObjCOptional { };

struct InControlCondition { };
struct Response { };
struct NamespacePackage { };
struct ModulePackage { };
struct DirectoryPackage { };

template <typename PackageT> std::string package_type_name();

template <typename T> struct diagram_source_t {
    diagram_source_t(clanguml::common::model::diagram_t dt, T &&s)
    {
        diagram_type = dt;
        src = std::move(s);
    }

    bool contains(std::string name) const;

    virtual std::string get_alias(std::string name) const
    {
        return "__INVALID_ALIAS__";
    }

    virtual std::string get_alias(std::string ns, std::string name) const
    {
        return get_alias(fmt::format("{}::{}", ns, name));
    }

    bool search(const std::string &pattern) const;

    int64_t find(const std::string &pattern, int64_t offset = 0) const;

    std::string to_string() const;

    T src;
    clanguml::common::model::diagram_t diagram_type;
    bool generate_packages{false};
};

struct plantuml_t : public diagram_source_t<std::string>,
                    util::memoized<plantuml_t, std::string, std::string> {

    using diagram_source_t::diagram_source_t;
    using source_type = std::string;
    using generator_tag = clanguml::common::generators::plantuml_generator_tag;

    inline static const std::string diagram_type_name{"PlantUML"};

    std::string get_alias(std::string name) const override;

    std::string get_alias_impl(std::string name) const;
};

struct mermaid_t : public diagram_source_t<std::string>,
                   util::memoized<mermaid_t, std::string, std::string> {
    using diagram_source_t::diagram_source_t;
    using source_type = std::string;
    using generator_tag = clanguml::common::generators::mermaid_generator_tag;

    inline static const std::string diagram_type_name{"MermaidJS"};

    std::string get_alias(std::string name) const override;

    std::string get_alias_class_diagram_impl(std::string name) const;

    std::string get_alias_sequence_diagram_impl(std::string name) const;

    std::string get_alias_impl(std::string name) const;
};

struct json_t : public diagram_source_t<nlohmann::json> {
    using diagram_source_t::diagram_source_t;
    using source_type = nlohmann::json;
    using generator_tag = clanguml::common::generators::json_generator_tag;

    inline static const std::string diagram_type_name{"JSON"};
};

struct graphml_t
    : public diagram_source_t<common::generators::graphml::graphml_t> {
    using diagram_source_t::diagram_source_t;
    using source_type = common::generators::graphml::graphml_t;
    using generator_tag = clanguml::common::generators::graphml_generator_tag;

    inline static const std::string diagram_type_name{"GraphML"};
};

std::optional<nlohmann::json> get_element_by_id(
    const nlohmann::json &j, const std::string &id);

std::optional<nlohmann::json> get_element(
    const nlohmann::json &j, const std::string &name);

std::optional<nlohmann::json> get_element(
    const nlohmann::json &j, const std::string &name, const std::string &type);

std::optional<nlohmann::json> get_element(
    const json_t &src, const std::string &name);

std::optional<nlohmann::json> get_participant(
    const nlohmann::json &j, const std::string &name);

nlohmann::json::const_iterator get_relationship(const nlohmann::json &j,
    const nlohmann::json &from, const nlohmann::json &to,
    const std::string &type, const std::string &label);

nlohmann::json::const_iterator get_relationship(const nlohmann::json &j,
    const std::string &from, const std::string &to, const std::string &type,
    const std::string &label = {});

std::string expand_name(const nlohmann::json &j, const std::string &name);

struct QualifiedName {
    QualifiedName(const char *n);
    QualifiedName(std::string_view n);
    QualifiedName(std::string_view ns_, std::string_view n);
    QualifiedName(const char *ns_, const char *n);

    operator std::string() const;

    std::string str(bool generate_packages = false) const;

    std::optional<std::string> ns;
    std::string name;
};

struct Message {
    template <typename... Attrs>
    Message(QualifiedName f, QualifiedName t, std::string m, Attrs &&...attrs)
        : from{std::move(f)}
        , to{std::move(t)}
        , message{std::move(m)}
        , is_static{has_type<Static, Attrs...>()}
        , is_incontrolcondition{has_type<InControlCondition, Attrs...>()}
        , is_response{has_type<Response, Attrs...>()}
        , is_cuda_kernel{has_type<CUDAKernel, Attrs...>()}
        , is_cuda_device{has_type<CUDADevice, Attrs...>()}
        , is_co_await{has_type<CoAwait, Attrs...>()}
        , is_co_yield{has_type<CoYield, Attrs...>()}
        , is_co_return{has_type<CoReturn, Attrs...>()}
        , is_coroutine{has_type<Coroutine, Attrs...>()}
    {
    }

    template <typename... Attrs>
    Message(Entrypoint &&e, QualifiedName t, std::string m, Attrs &&...attrs)
        : Message(QualifiedName{""}, std::move(t), {},
              std::forward<Attrs>(attrs)...)
    {
        is_entrypoint = true;
    }

    template <typename... Attrs>
    Message(Exitpoint &&e, QualifiedName t, Attrs &&...attrs)
        : Message(QualifiedName{""}, std::move(t), {},
              std::forward<Attrs>(attrs)...)
    {
        is_exitpoint = true;
    }

    QualifiedName from;
    QualifiedName to;
    std::string message;
    std::optional<std::string> return_type;

    bool is_static{false};
    bool is_entrypoint{false};
    bool is_exitpoint{false};
    bool is_incontrolcondition{false};
    bool is_response{false};
    bool is_cuda_kernel{false};
    bool is_cuda_device{false};
    bool is_co_await{false};
    bool is_co_yield{false};
    bool is_co_return{false};
    bool is_coroutine{false};
};

//
// JSON test helpers
//
struct File {
    explicit File(const std::string &f);

    const std::string file;
};

namespace json_helpers {
int find_message_nested(const nlohmann::json &j, const std::string &from,
    const std::string &to, const Message &msg, const nlohmann::json &from_p,
    const nlohmann::json &to_p, int &count, const int64_t offset,
    std::optional<int32_t> chain_index = {});

int find_message_impl(const nlohmann::json &j, const std::string &from,
    const std::string &to, const Message &msg, int64_t offset,
    std::optional<int32_t> chain_index = {});

int64_t find_message(const nlohmann::json &j, const File &from, const File &to,
    const Message &msg, int64_t offset);

int64_t find_message(const nlohmann::json &j, const std::string &from,
    const std::string &to, const Message &msg, int64_t offset = 0);

int64_t find_message_in_chain(const nlohmann::json &j, const std::string &from,
    const std::string &to, const Message &msg, int64_t offset = 0,
    uint32_t chain_index = 0);

} // namespace json_helpers

int64_t find_message_in_chain(const json_t &d, const Message &msg,
    int64_t offset, bool fail, uint32_t chain_index);

std::string expand_name(const graphml_t &d, const std::string &name);

std::string get_attr_key_id(
    const graphml_t &d, std::string const &for_, std::string const &name);

pugi::xpath_node get_element(const graphml_t &d, const QualifiedName &cls);

pugi::xml_node get_element(
    const graphml_t &d, const pugi::xml_node &parent, const QualifiedName &cls);

pugi::xpath_node get_graph_node(
    const graphml_t &d, const pugi::xml_node &parent, const std::string &name);

pugi::xpath_node get_graph(
    const graphml_t &d, const pugi::xml_node &parent, const std::string &name);

pugi::xpath_node get_element(
    const graphml_t &d, const std::string &type, const QualifiedName &cls);

pugi::xpath_node get_element(const graphml_t &d,
    const std::vector<std::string> &types, const QualifiedName &cls);

pugi::xml_node get_nested_element(
    const graphml_t &d, const std::vector<std::string> &p);

bool has_data(const graphml_t &d, const pugi::xml_node &node,
    const std::string &data, const std::string &value);

pugi::xml_node get_file_node(const graphml_t &d, const pugi::xml_node &parent,
    const std::filesystem::path &path_fs);

pugi::xpath_node get_relationship(const graphml_t &d,
    const std::string &from_id, const std::string &to_id,
    const std::string &relationship_type, const std::string &label = "",
    const std::string &access = "");

bool find_relationship(const graphml_t &d, const std::string &from_id,
    const std::string &to_id, const std::string &relationship_type);

} // namespace clanguml::test