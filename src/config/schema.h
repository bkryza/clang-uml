/**
 * @file src/config/schema.h
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
#pragma once

#include <string>

namespace clanguml::config {

const std::string schema_str = R"(
types:
    map_t<K;V>: { $K: V }
    comment_parser_t: !variant
        - plain
        - clang
    diagram_type_t: !variant
        - class
        - sequence
        - include
        - package
    generate_method_arguments_t: !variant
        - full
        - abbreviated
        - none
    generate_links_t:
        link: string
        tooltip: string
    git_t:
        branch: string
        revision: [string, int]
        commit: string
        toplevel: string
    layout_hint_key: !variant
        - up
        - left
        - right
        - down
        - row
        - column
        - together
    layout_hint_value: [string, [string]]
    layout_hint_t: [map_t<layout_hint_key;layout_hint_value>]
    layout_t: map_t<string;layout_hint_t>
    package_type_t: !variant
        - namespace
        - directory
        - module
    member_order_t: !variant
        - lexical
        - as_is
    regex_t:
        r: string
    regex_or_string_t: [string, regex_t]
    element_types_filter_t: !variant
        - class
        - enum
        - concept
        - method
        - function
        - function_template
        - lambda
    relationship_filter_t: !variant
        - extension
        - inheritance
        - composition
        - aggregation
        - containment
        - ownership
        - association
        - instantiation
        - friendship
        - alias
        - dependency
        - constraint
        - none
    access_filter_t: !variant
        - public
        - protected
        - private
    module_access_filter_t: !variant
        - public
        - private
    method_type_filter_t: !variant
        - constructor
        - destructor
        - assignment
        - operator
        - defaulted
        - deleted
        - static
    callee_type_filter_t: !variant
        - constructor
        - assignment
        - operator
        - defaulted
        - static
        - method
        - function
        - function_template
        - lambda
        - cuda_kernel
        - cuda_device
    context_filter_match_t:
        match:
            radius: int
            pattern: regex_or_string_t
    context_filter_t:
        - regex_or_string_t
        - context_filter_match_t
    filter_t:
        namespaces: !optional [regex_or_string_t]
        modules: !optional [regex_or_string_t]
        elements: !optional [regex_or_string_t]
        element_types: !optional [element_types_filter_t]
        relationships: !optional [relationship_filter_t]
        access: !optional [access_filter_t]
        module_access: !optional [module_access_filter_t]
        subclasses: !optional [regex_or_string_t]
        parents: !optional [regex_or_string_t]
        specializations: !optional [regex_or_string_t]
        dependants: !optional [regex_or_string_t]
        dependencies: !optional [regex_or_string_t]
        context: !optional [context_filter_t]
        paths: !optional [string]
        method_types: !optional [method_type_filter_t]
        callee_types: !optional [callee_type_filter_t]
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
        comment_parser: !optional comment_parser_t
        debug_mode: !optional bool
        exclude: !optional filter_t
        generate_links: !optional generate_links_t
        git: !optional git_t
        glob: !optional [string]
        include: !optional filter_t
        plantuml: !optional
            before: !optional [string]
            after: !optional [string]
            cmd: !optional string
            style: !optional map_t<string;string>
        mermaid: !optional
            before: !optional [string]
            after: !optional [string]
            cmd: !optional string
        relative_to: !optional string
        using_namespace: !optional [string, [string]]
        using_module: !optional string
        generate_metadata: !optional bool
        title: !optional string
        #
        # Class diagram specific options
        #
        generate_method_arguments: !optional generate_method_arguments_t
        generate_packages: !optional bool
        generate_concept_requirements: !optional bool
        package_type: !optional package_type_t
        generate_template_argument_dependencies: !optional bool
        skip_redundant_dependencies: !optional bool
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
        comment_parser: !optional comment_parser_t
        debug_mode: !optional bool
        exclude: !optional filter_t
        generate_links: !optional generate_links_t
        git: !optional git_t
        glob: !optional [string]
        include: !optional filter_t
        plantuml: !optional
            before: !optional [string]
            after: !optional [string]
            cmd: !optional string
        mermaid: !optional
            before: !optional [string]
            after: !optional [string]
            cmd: !optional string
        relative_to: !optional string
        type_aliases: !optional map_t<string;string>
        using_namespace: !optional [string, [string]]
        generate_metadata: !optional bool
        title: !optional string
        #
        # Sequence diagram specific options
        #
        generate_method_arguments: !optional generate_method_arguments_t
        combine_free_functions_into_file_participants: !optional bool
        inline_lambda_messages: !optional bool
        generate_return_types: !optional bool
        generate_condition_statements: !optional bool
        generate_message_comments: !optional bool
        message_comment_width: !optional int
        participants_order: !optional [string]
        start_from: !optional [source_location_t] # deprecated -> 'from'
        from: !optional [source_location_t]
        from_to: !optional [[source_location_t]]
        to: !optional [source_location_t]
    package_diagram_t:
        type: !variant [package]
        #
        # Common options
        #
        __parent_path: !optional string
        comment_parser: !optional comment_parser_t
        debug_mode: !optional bool
        exclude: !optional filter_t
        generate_links: !optional generate_links_t
        git: !optional git_t
        glob: !optional [string]
        include: !optional filter_t
        plantuml: !optional
            before: !optional [string]
            after: !optional [string]
            cmd: !optional string
        mermaid: !optional
            before: !optional [string]
            after: !optional [string]
            cmd: !optional string
        relative_to: !optional string
        using_namespace: !optional [string, [string]]
        using_module: !optional string
        generate_metadata: !optional bool
        title: !optional string
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
        comment_parser: !optional comment_parser_t
        debug_mode: !optional bool
        exclude: !optional filter_t
        generate_links: !optional generate_links_t
        git: !optional git_t
        glob: !optional [string]
        include: !optional filter_t
        plantuml: !optional
            before: !optional [string]
            after: !optional [string]
            cmd: !optional string
        mermaid: !optional
            before: !optional [string]
            after: !optional [string]
            cmd: !optional string
        relative_to: !optional string
        using_namespace: !optional [string, [string]]
        generate_metadata: !optional bool
        title: !optional string
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
    query_driver: !optional string
    add_compile_flags: !optional [string]
    remove_compile_flags: !optional [string]
    allow_empty_diagrams: !optional bool
    diagram_templates: !optional diagram_templates_t
    diagrams: !required map_t<string;diagram_t>
    #
    # Common options
    #
    __parent_path: !optional string
    comment_parser: !optional comment_parser_t
    debug_mode: !optional bool
    exclude: !optional filter_t
    generate_links: !optional generate_links_t
    git: !optional git_t
    glob: !optional [string]
    include: !optional filter_t
    plantuml: !optional
        before: !optional [string]
        after: !optional [string]
        cmd: !optional string
    mermaid: !optional
        before: !optional [string]
        after: !optional [string]
        cmd: !optional string
    relative_to: !optional string
    using_namespace: !optional [string, [string]]
    using_module: !optional string
    generate_metadata: !optional bool
    #
    # Inheritable custom options
    #
    include_relations_also_as_members: !optional bool
    generate_method_arguments: !optional generate_method_arguments_t
    combine_free_functions_into_file_participants: !optional bool
    inline_lambda_messages: !optional bool
    generate_concept_requirements: !optional bool
    generate_return_types: !optional bool
    generate_condition_statements: !optional bool
    generate_message_comments: !optional bool
    message_comment_width: !optional int
    generate_packages: !optional bool
    group_methods: !optional bool
    package_type: !optional package_type_t
    generate_template_argument_dependencies: !optional bool
    skip_redundant_dependencies: !optional bool
    type_aliases: !optional map_t<string;string>
)";

} // namespace clanguml::config