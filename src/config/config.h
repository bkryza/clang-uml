/**
 * @file src/config/config.h
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
#pragma once

#include "class_diagram/model/diagram.h"
#include "common/model/enums.h"
#include "common/types.h"
#include "option.h"
#include "util/util.h"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <variant>
#include <vector>

namespace clanguml {

namespace cli {
struct runtime_config;
} // namespace cli

/**
 * @brief Configuration file related classes
 *
 * \image html config_class.svg
 */
namespace config {

/*! Select how the method arguments should be rendered */
enum class method_arguments {
    full,        /*! Full */
    abbreviated, /*! Abrreviate, string between '(' and ')' is cropped */
    none         /*! Empty string between '(' and ')' */
};

std::string to_string(method_arguments ma);

/*! Types of methods, which can be used in diagram filters */
enum class method_type {
    constructor,
    destructor,
    assignment,
    operator_,
    defaulted,
    deleted,
    static_
};

std::string to_string(method_type mt);

/*! Types of call expressions, which can be used in sequence diagram filters */
enum class callee_type {
    constructor,
    assignment,
    operator_,
    defaulted,
    static_,
    method,
    function,
    function_template,
    lambda
};

std::string to_string(callee_type mt);

/*! How packages in diagrams should be generated */
enum class package_type_t {
    kNamespace, /*!< From namespaces */
    kDirectory  /*!< From directories */
};

std::string to_string(package_type_t mt);

/*! How class methods and members should be ordered in diagrams */
enum class member_order_t {
    lexical, /*! Lexical order based on entire method or member signature
                without type */
    as_is    /*! As written in source code */
};

std::string to_string(member_order_t mt);

/*! Which comment parser should be used */
enum class comment_parser_t {
    plain, /*!< Basic string parser */
    clang  /*!< Clang's internal comment parser with structured output */
};

std::string to_string(comment_parser_t cp);

/**
 * @brief PlantUML diagram config section
 *
 * This configuration option can be used to add any PlantUML directives
 * before or after the generated diagram, such as diagram name, or custom
 * notes.
 */
struct plantuml {
    /*! List of directives to add before diagram */
    std::vector<std::string> before;
    /*! List of directives to add before diagram */
    std::vector<std::string> after;

    void append(const plantuml &r);
};

/**
 * @brief Definition of diagram template
 */
struct diagram_template {
    /*! Template description */
    std::string description;

    /*! Diagram type */
    common::model::diagram_t type{common::model::diagram_t::kClass};

    /*! Template which will be processed by Inja to generate the actual
     * diagram
     */
    std::string jinja_template;
};

struct filter {
    /*! @brief Namespaces filter
     *
     * Example:
     *
     * ```yaml
     *   include
     *     namespaces:
     *       - ns1::ns2
     *       - r: ".*detail.*"
     * ```
     */
    std::vector<common::namespace_or_regex> namespaces;

    /*! @brief Elements filter
     *
     * Example:
     *
     * ```yaml
     *   exclude:
     *     elements:
     *       - ns1::ns2::ClassA
     *       - r: ".*Enum.*"
     * ```
     */
    std::vector<common::string_or_regex> elements;

    /*! @brief Element types filter
     *
     * This filter allows to filter diagram elements based on their type.
     *
     * ```yaml
     *   include:
     *     elements:
     *       - class
     *       - concept
     * ```
     */
    std::vector<std::string> element_types;

    /*! @brief Relationships filter
     *
     * This filter allows to filter relationships included in the diagram.
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     relationships:
     *       - inheritance
     * ```
     */
    std::vector<common::model::relationship_t> relationships;

    /*! @brief Access type filter
     *
     * This filter allows to filter class members methods based on their access:
     *  - public
     *  - protected
     *  - private
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     relationships:
     *       - inheritance
     * ```
     */
    std::vector<common::model::access_t> access;

    /*! @brief Subclasses filter
     *
     * This filter allows to filter classes based on having a specified parent.
     * This filter also matches the specified parent itself.
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     subclasses:
     *       - ns1::ns2::ClassA
     * ```
     */
    std::vector<common::string_or_regex> subclasses;

    /*! @brief Parents filter
     *
     * This filter allows to filter classes based on being a base class of a
     * specified child.
     * This filter also matches the specified child itself.
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     parents:
     *       - ns1::ns2::ChildA
     * ```
     */
    std::vector<common::string_or_regex> parents;

    /*! @brief Specializations filter
     *
     * This filter allows to filter class template specializations of a specific
     * template.
     * This filter also matches the specified template itself.
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     specializations:
     *       - ns1::ns2::ClassA<int,T>
     * ```
     */
    std::vector<common::string_or_regex> specializations;

    /*! @brief Dependants filter
     *
     * This filter allows to filter classes based on whether they are a
     * dependant on a specified class.
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     dependants:
     *       - ns1::ns2::ClassA
     * ```
     */
    std::vector<common::string_or_regex> dependants;

    /*! @brief Dependencies filter
     *
     * This filter allows to filter classes based on whether they are a
     * dependency of a specified class.
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     dependencies:
     *       - ns1::ns2::ClassA
     * ```
     */
    std::vector<common::string_or_regex> dependencies;

    /*! @brief Context filter
     *
     * This filter allows to filter classes based on whether they have at least
     * one direct relation to the specified class.
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     context:
     *       - ns1::ns2::ClassA
     * ```
     */
    std::vector<common::string_or_regex> context;

    /*! @brief Paths filter
     *
     * This filter allows to filter diagram elements based on the source
     * location of their declaration.
     *
     * Example:
     *
     * ```yaml
     *   include:
     *     context:
     *       - ns1::ns2::ClassA
     * ```
     */
    std::vector<std::string> paths;

    /*! @brief Method types filter
     *
     * This filter allows to filter class methods based on their category.
     *
     * @see method_type
     *
     * Example:
     *
     * ```yaml
     *   exclude:
     *     method_types:
     *       - constructor
     *       - operator
     *       - assignment
     * ```
     */
    std::vector<method_type> method_types;

    /*! @brief Callee types filter
     *
     * This filter allows to filter sequence diagram calls by callee types.
     *
     * @see method_type
     *
     * Example:
     *
     * ```yaml
     *   exclude:
     *     callee_types:
     *       - constructor
     *       - operator
     * ```
     */
    std::vector<callee_type> callee_types;
};

enum class hint_t { up, down, left, right, together, row, column };

std::string to_string(hint_t t);

struct layout_hint {
    hint_t hint{hint_t::up};
    std::variant<std::string, std::vector<std::string>> entity;
};

using layout_hints = std::map<std::string, std::vector<layout_hint>>;

struct generate_links_config {
    std::string link;
    std::string tooltip;
};

struct git_config {
    std::string branch;
    std::string revision;
    std::string commit;
    std::string toplevel;
};

struct relationship_hint_t {
    std::map<unsigned int, common::model::relationship_t> argument_hints;
    common::model::relationship_t default_hint;

    relationship_hint_t(common::model::relationship_t def =
                            common::model::relationship_t::kAggregation)
        : default_hint{def}
    {
    }

    common::model::relationship_t get(unsigned int argument_index) const
    {
        if (argument_hints.count(argument_index) > 0)
            return argument_hints.at(argument_index);

        return default_hint;
    }
};

using relationship_hints_t = std::map<std::string, relationship_hint_t>;

using type_aliases_t = std::map<std::string, std::string>;
using type_aliases_longer_first_t =
    std::map<std::string, std::string, std::greater<>>;

enum class location_t { marker, fileline, function };

std::string to_string(location_t cp);

struct source_location {
    location_t location_type{location_t::function};
    std::string location;
};

/**
 * @brief Represents subset of inheritable configuration options
 *
 * This class contains a subset of configuration options, which are inherited
 * from the top level of the configuration to the diagrams.
 *
 * @embed{inheritable_diagram_options_context_class.svg}
 */
struct inheritable_diagram_options {
    virtual ~inheritable_diagram_options() = default;

    option<std::vector<std::string>> glob{"glob"};
    option<common::model::namespace_> using_namespace{"using_namespace"};
    option<bool> include_relations_also_as_members{
        "include_relations_also_as_members", true};
    option<filter> include{"include"};
    option<filter> exclude{"exclude"};
    option<plantuml> puml{"plantuml", option_inherit_mode::kAppend};
    option<method_arguments> generate_method_arguments{
        "generate_method_arguments", method_arguments::full};
    option<bool> group_methods{"group_methods", true};
    option<member_order_t> member_order{
        "member_order", member_order_t::lexical};
    option<bool> generate_packages{"generate_packages", false};
    option<package_type_t> package_type{
        "package_type", package_type_t::kNamespace};
    option<bool> generate_template_argument_dependencies{
        "generate_template_argument_dependencies", true};
    option<bool> skip_redundant_dependencies{
        "skip_redundant_dependencies", true};
    option<generate_links_config> generate_links{"generate_links"};
    option<git_config> git{"git"};
    option<layout_hints> layout{"layout"};
    // This is the absolute filesystem path to the directory containing
    // the current .clang-uml config file - it is set automatically
    option<std::filesystem::path> base_directory{"__parent_path"};
    // This is the relative path with respect to the `base_directory`,
    // against which all matches are made, if not provided it defaults to
    // the `base_directory`
    option<std::filesystem::path> relative_to{"relative_to"};
    option<bool> generate_system_headers{"generate_system_headers", false};
    option<relationship_hints_t> relationship_hints{"relationship_hints"};
    option<type_aliases_t> type_aliases{"type_aliases"};
    option<comment_parser_t> comment_parser{
        "comment_parser", comment_parser_t::plain};
    option<bool> combine_free_functions_into_file_participants{
        "combine_free_functions_into_file_participants", false};
    option<bool> generate_return_types{"generate_return_types", false};
    option<bool> generate_condition_statements{
        "generate_condition_statements", false};
    option<std::vector<std::string>> participants_order{"participants_order"};
    option<bool> debug_mode{"debug_mode", false};
    option<bool> generate_metadata{"generate_metadata", true};

    void inherit(const inheritable_diagram_options &parent);

    std::string simplify_template_type(std::string full_name) const;
};

/**
 * @brief Common diagram configuration type
 *
 * This class provides common interface for diagram configuration sections
 * of different diagram types.
 *
 * @embed{diagram_config_hierarchy_class.svg}
 */
struct diagram : public inheritable_diagram_options {
    ~diagram() override = default;

    virtual common::model::diagram_t type() const = 0;

    /**
     * @brief Returns list of translation unit paths
     *
     * @return List of translation unit paths
     */
    std::vector<std::string> get_translation_units() const;

    /**
     * @brief Make path relative to the `relative_to` config option
     *
     * @param p Input path
     * @return Relative path
     */
    std::filesystem::path make_path_relative(
        const std::filesystem::path &p) const;

    /**
     * @brief Returns absolute path of the `relative_to` option
     *
     * @return Absolute path of `relative_to`
     */
    std::filesystem::path root_directory() const;

    std::optional<std::string> get_together_group(
        const std::string &full_name) const;

    /**
     * @brief Initialize predefined set of C++ type aliases
     *
     * This method is responsible for setting up a predefined set of C++
     * aliases to make the diagrams look nicer, for instance we want to have
     * `std::string` instead of `std::basic_string<char>`.
     */
    void initialize_type_aliases();

    std::string name;
};

/**
 * @brief Class diagram configuration
 */
struct class_diagram : public diagram {
    ~class_diagram() override = default;

    common::model::diagram_t type() const override;

    void initialize_relationship_hints();
};

/**
 * @brief Sequence diagram configuration
 */
struct sequence_diagram : public diagram {
    ~sequence_diagram() override = default;

    common::model::diagram_t type() const override;

    option<std::vector<source_location>> start_from{"start_from"};
};

/**
 * @brief Package diagram configuration
 */
struct package_diagram : public diagram {
    ~package_diagram() override = default;

    common::model::diagram_t type() const override;
};

/**
 * @brief Include diagram configuration
 */
struct include_diagram : public diagram {
    ~include_diagram() override = default;

    common::model::diagram_t type() const override;
};

/**
 * @brief Represents entire configuration file
 *
 * This class contains all information loaded from the provided `clang-uml`
 * configuration file, along with information inferred from the system
 * such as current directory or Git context.
 *
 * The options in this class are not inheritable into specific diagram
 * configurations.
 *
 * @embed{config_context_class.svg}
 *
 * The configuration file is loaded by invoking @see clanguml::config::load()
 * function.
 */
struct config : public inheritable_diagram_options {
    /*! Path to the directory containing compile_commands.json */
    option<std::string> compilation_database_dir{
        "compilation_database_dir", "."};

    /*! List of compilation flags to be injected into the compilation
     * commands from database
     */
    option<std::vector<std::string>> add_compile_flags{"add_compile_flags"};

    /*! List of compilation flags to be removed from the compilation
     * commands from database
     */
    option<std::vector<std::string>> remove_compile_flags{
        "remove_compile_flags"};

    /*! Extract include paths by executing specified compiler driver.
     * When the value is `'`, the command specified in compile_commands.json
     * will be used. Otherwise the provided value will be treated as
     * compiler executable (e.g. clang++-15) and executed.
     */
    option<std::string> query_driver{"query_driver"};

    /*! Diagrams output directory */
    option<std::string> output_directory{"output_directory"};

    /*! Dictionary of user defined diagram templates, if any */
    option<std::map<std::string, diagram_template>> diagram_templates{
        "diagram_templates"};

    /*! Dictionary of diagrams, keys represent diagram names */
    std::map<std::string, std::shared_ptr<diagram>> diagrams;

    /**
     * Initialize predefined diagram templates.
     */
    void initialize_diagram_templates();
};

/**
 * @brief Load and parse `.clang-uml` configuration file
 *
 * This function takes the path to the configuration file and some options,
 * parses the YAML file and creates a @see clanguml::config::config instance.
 *
 * @embed{load_config_sequence.svg}
 *
 * @param config_file Path to the configuration file
 * @param paths_relative_to_pwd Whether the paths in the configuration file
 *                              should be relative to the parent directory of
 *                              the configuration file or to the current
 *                              directory (`$PWD`)
 * @param no_metadata Whether the diagram should skip metadata at the end
 * @param validate If true, perform schema validation
 * @return Configuration instance
 */
config load(const std::string &config_file,
    std::optional<bool> paths_relative_to_pwd = {},
    std::optional<bool> no_metadata = {}, bool validate = true);

} // namespace config

namespace config {
/** @defgroup yaml_emitters YAML serialization emitters
 *  Overloads for YAML serializations for various config types.
 *  @{
 */
YAML::Emitter &operator<<(YAML::Emitter &out, const config &c);

YAML::Emitter &operator<<(
    YAML::Emitter &out, const inheritable_diagram_options &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const filter &f);

YAML::Emitter &operator<<(YAML::Emitter &out, const plantuml &p);

YAML::Emitter &operator<<(YAML::Emitter &out, const method_arguments &ma);

YAML::Emitter &operator<<(YAML::Emitter &out, const generate_links_config &glc);

YAML::Emitter &operator<<(YAML::Emitter &out, const git_config &gc);

YAML::Emitter &operator<<(YAML::Emitter &out, const relationship_hint_t &rh);

YAML::Emitter &operator<<(YAML::Emitter &out, const comment_parser_t &cp);

YAML::Emitter &operator<<(YAML::Emitter &out, const hint_t &h);

YAML::Emitter &operator<<(YAML::Emitter &out, const class_diagram &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const sequence_diagram &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const include_diagram &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const package_diagram &c);

YAML::Emitter &operator<<(YAML::Emitter &out, const layout_hint &c);

#ifdef _MSC_VER
YAML::Emitter &operator<<(YAML::Emitter &out, const std::filesystem::path &p);

YAML::Emitter &operator<<(
    YAML::Emitter &out, const std::vector<std::filesystem::path> &p);
#endif

YAML::Emitter &operator<<(YAML::Emitter &out, const source_location &sc);

template <typename T>
YAML::Emitter &operator<<(YAML::Emitter &out, const option<T> &o)
{
    if (o.has_value) {
        out << YAML::Key << o.name;
        if constexpr (std::is_same_v<T, std::filesystem::path>)
            out << YAML::Value << o.value.string();
        else
            out << YAML::Value << o.value;
    }
    return out;
}
/** @} */ // end of yaml_emitters

} // namespace config

namespace common::model {
/** @addtogroup yaml_emitters YAML serialization emitters
 *  Overloads for YAML serializations for various model types.
 *  @{
 */
YAML::Emitter &operator<<(YAML::Emitter &out, const namespace_ &n);
YAML::Emitter &operator<<(YAML::Emitter &out, const relationship_t &r);
YAML::Emitter &operator<<(YAML::Emitter &out, const access_t &r);
YAML::Emitter &operator<<(YAML::Emitter &out, const diagram_t &d);
/** @} */ // end of yaml_emitters
} // namespace common::model

} // namespace clanguml
