/**
 * @file src/config/schema.h
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

#include <string>

namespace clanguml::config {

const std::string schema_str = R"(
types:
    map_t<K;V>: { $K: V }
    comment_parser_t: !variant [plain, clang]
    diagram_type_t: !variant [class, sequence, include, package]
    generate_method_arguments_t: !variant [full, abbreviated, none]
    generate_links_t:
        link: string
        tooltip: string
    git_t: map_t<string;string>
    layout_hint_key: !variant [up, left, right, down, row, column, together]
    layout_hint_value: [string, [string]]
    layout_hint_t: [map_t<layout_hint_key;layout_hint_value>]
    layout_t: map_t<string;layout_hint_t>
    package_type_t: !variant [namespace, directory]
    member_order_t: !variant [lexical, as_is]
    regex_t:
        r: string
    regex_or_string_t: [string, regex_t]
    namespaces_filter_t: regex_or_string_t
    elements_filter_t: regex_or_string_t
    function_location_t:
        function: string
    marker_location_t:
        marker: string
    source_location_t:
        - function_location_t
        - marker_location_t
    class_diagram_t:
        type: !variant [class]
        #
        # Common options
        #
        __parent_path: !optional string
        base_directory: !optional string
        comment_parser: !optional comment_parser_t
        debug_mode: !optional bool
        exclude: !optional map_t<string;any>
        generate_links: !optional generate_links_t
        git: !optional git_t
        glob: !optional [string]
        include: !optional map_t<string;any>
        plantuml: !optional
            before: !optional [string]
            after: !optional [string]
        relative_to: !optional string
        using_namespace: !optional [string, [string]]
        generate_metadata: !optional bool
        #
        # Class diagram specific options
        #
        generate_method_arguments: !optional generate_method_arguments_t
        generate_packages: !optional bool
        package_type: !optional package_type_t
        method_order: !optional member_order_t
        member_order: !optional member_order_t
        group_methods: !optional bool
        type_aliases: !optional map_t<string;string>
        relationship_hints: !optional map_t<string;any>
        include_relations_also_as_members: !optional bool
        layout: !optional layout_t
    sequence_diagram_t:
        type: !variant [sequence]
        #
        # Common options
        #
        __parent_path: !optional string
        base_directory: !optional string
        comment_parser: !optional comment_parser_t
        debug_mode: !optional bool
        exclude: !optional map_t<string;any>
        generate_links: !optional generate_links_t
        git: !optional git_t
        glob: !optional [string]
        include: !optional map_t<string;any>
        plantuml: !optional
            before: !optional [string]
            after: !optional [string]
        relative_to: !optional string
        using_namespace: !optional [string, [string]]
        generate_metadata: !optional bool
        #
        # Sequence diagram specific options
        #
        generate_method_arguments: !optional generate_method_arguments_t
        combine_free_functions_into_file_participants: !optional bool
        generate_return_types: !optional bool
        generate_condition_statements: !optional bool
        participants_order: !optional [string]
        start_from: !optional [source_location_t]
    package_diagram_t:
        type: !variant [package]
        #
        # Common options
        #
        __parent_path: !optional string
        base_directory: !optional string
        comment_parser: !optional comment_parser_t
        debug_mode: !optional bool
        exclude: !optional map_t<string;any>
        generate_links: !optional generate_links_t
        git: !optional git_t
        glob: !optional [string]
        include: !optional map_t<string;any>
        plantuml: !optional
            before: !optional [string]
            after: !optional [string]
        relative_to: !optional string
        using_namespace: !optional [string, [string]]
        generate_metadata: !optional bool
        #
        # Package diagram specific options
        #
        generate_packages: !optional bool
        package_type: !optional package_type_t
        layout: !optional layout_t
    include_diagram_t:
        type: !variant [include]
        #
        # Common options
        #
        __parent_path: !optional string
        base_directory: !optional string
        comment_parser: !optional comment_parser_t
        debug_mode: !optional bool
        exclude: !optional map_t<string;any>
        generate_links: !optional generate_links_t
        git: !optional git_t
        glob: !optional [string]
        include: !optional map_t<string;any>
        plantuml: !optional
            before: !optional [string]
            after: !optional [string]
        relative_to: !optional string
        using_namespace: !optional [string, [string]]
        generate_metadata: !optional bool
        #
        # Include diagram specific options
        #
        generate_system_headers: !optional bool
    diagram_t:
        - class_diagram_t
        - sequence_diagram_t
        - package_diagram_t
        - include_diagram_t
    diagram_template_t:
        description: !optional string
        type: diagram_type_t
        template: string
    diagram_templates_t: map_t<string;diagram_template_t>

root:
    #
    # Root options
    #
    compilation_database_dir: !optional string
    output_directory: !optional string
    add_compile_flags: !optional [string]
    remove_compile_flags: !optional [string]
    diagram_templates: !optional diagram_templates_t
    diagrams: !required map_t<string;diagram_t>
    #
    # Common options
    #
    __parent_path: !optional string
    base_directory: !optional string
    comment_parser: !optional comment_parser_t
    debug_mode: !optional bool
    exclude: !optional map_t<string;any>
    generate_links: !optional generate_links_t
    git: !optional git_t
    glob: !optional [string]
    include: !optional map_t<string;any>
    plantuml: !optional
        before: !optional [string]
        after: !optional [string]
    relative_to: !optional string
    using_namespace: !optional [string, [string]]
    generate_metadata: !optional bool
    #
    # Inheritable custom options
    #
    include_relations_also_as_members: !optional bool
    generate_method_arguments: !optional generate_method_arguments_t
    combine_free_functions_into_file_participants: !optional bool
    generate_return_types: !optional bool
    generate_condition_statements: !optional bool
    generate_packages: !optional bool
    group_methods: !optional bool
    package_type: !optional package_type_t
)";

} // namespace clanguml::config