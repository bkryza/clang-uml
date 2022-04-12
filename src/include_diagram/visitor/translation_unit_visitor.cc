/**
 * src/include_diagram/visitor/translation_unit_visitor.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

#include "translation_unit_visitor.h"

#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_preprocessor.hpp>

#include <filesystem>

namespace clanguml::include_diagram::visitor {

translation_unit_visitor::translation_unit_visitor(
    cppast::cpp_entity_index &idx,
    clanguml::include_diagram::model::diagram &diagram,
    const clanguml::config::include_diagram &config)
    : ctx{idx, diagram, config}
{
}

void translation_unit_visitor::operator()(const cppast::cpp_entity &file)
{
    assert(file.kind() == cppast::cpp_entity_kind::file_t);

    process_source_file(static_cast<const cppast::cpp_file &>(file));

    cppast::visit(file,
        [&, this](const cppast::cpp_entity &e, cppast::visitor_info info) {
            if (e.kind() == cppast::cpp_entity_kind::include_directive_t) {
                const auto &inc =
                    static_cast<const cppast::cpp_include_directive &>(e);

                process_include_directive(inc);
            }
        });
}

void translation_unit_visitor::process_include_directive(
    const cppast::cpp_include_directive &include_directive)
{
    using common::model::relationship;
    using common::model::source_file;
    using common::model::source_file_t;

    assert(ctx.get_current_file().has_value());

    LOG_DBG("Processing include directive {} in file {}",
        include_directive.full_path(), ctx.get_current_file().value().name());

    auto include_path = std::filesystem::path(include_directive.full_path());

    // Make sure the file_path is absolute with respect to the
    // filesystem, and in normal form
    if (include_path.is_relative()) {
        include_path = ctx.config().base_directory() / include_path;
    }

    include_path = include_path.lexically_normal();

    if (ctx.diagram().should_include(source_file{include_path})) {
        // Relativize the path with respect to relative_to config option
        auto relative_include_path = include_path;
        if (ctx.config().relative_to) {
            const std::filesystem::path relative_to{ctx.config().relative_to()};
            relative_include_path =
                std::filesystem::relative(include_path, relative_to);
        }

        // Check if this source file is already registered in the diagram,
        // if not add it
        auto diagram_path = source_file{relative_include_path}.full_path();
        if (!ctx.diagram().get_element(diagram_path).has_value()) {
            ctx.diagram().add_file(
                std::make_unique<source_file>(relative_include_path));
        }

        auto &include_file = ctx.diagram().get_element(diagram_path).value();

        include_file.set_type(source_file_t::kHeader);

        // Add relationship from the currently parsed source file to this
        // include file
        auto relationship_type = common::model::relationship_t::kAssociation;
        if (include_directive.include_kind() ==
            cppast::cpp_include_kind::system)
            relationship_type = common::model::relationship_t::kDependency;

        ctx.get_current_file().value().add_relationship(
            relationship{relationship_type, include_file.alias()});

        include_file.set_file(
            std::filesystem::absolute(include_directive.full_path())
                .lexically_normal()
                .string());
        include_file.set_line(0);
    }
    else {
        LOG_DBG("Skipping include directive to file {}", include_path.string());
    }
}

void translation_unit_visitor::process_source_file(const cppast::cpp_file &file)
{
    using common::model::relationship;
    using common::model::source_file;
    using common::model::source_file_t;

    LOG_DBG("Processing source file {}", file.name());

    auto file_path = std::filesystem::path(file.name());

    // Make sure the file_path is absolute with respect to the
    // filesystem, and in normal form
    if (file_path.is_relative()) {
        file_path = ctx.config().base_directory() / file_path;
    }

    file_path = file_path.lexically_normal();

    if (ctx.diagram().should_include(source_file{file_path})) {
        // Relativize the path with respect to relative_to config option
        auto relative_file_path = file_path;
        if (ctx.config().relative_to) {
            const std::filesystem::path relative_to{ctx.config().relative_to()};
            relative_file_path =
                std::filesystem::relative(file_path, relative_to);
        }

        // Check if this source file is already registered in the diagram,
        // if not add it
        auto diagram_path = source_file{relative_file_path}.full_path();
        if (!ctx.diagram().get_element(diagram_path).has_value()) {
            ctx.diagram().add_file(
                std::make_unique<source_file>(relative_file_path));
        }

        auto &source_file = ctx.diagram().get_element(diagram_path).value();

        const std::string implementation_suffix_prefix{".c"};
        if (file_path.has_extension() &&
            util::starts_with(
                file_path.extension().string(), implementation_suffix_prefix)) {
            source_file.set_type(source_file_t::kImplementation);
        }
        else
            source_file.set_type(source_file_t::kHeader);

        ctx.set_current_file(type_safe::opt_ref(source_file));
    }
    else {
        LOG_DBG("Skipping source file {}", file_path.string());
    }
}
}
