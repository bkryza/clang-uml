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

    const auto &f = static_cast<const cppast::cpp_file &>(file);

    auto file_path = f.name();

    LOG_DBG("Processing source file {}", file_path);

    process_file(file_path, true);

    cppast::visit(file,
        [&, this](const cppast::cpp_entity &e, cppast::visitor_info info) {
            if (e.kind() == cppast::cpp_entity_kind::include_directive_t) {
                assert(ctx.get_current_file().has_value());

                auto file_path_cpy = file.name();

                const auto &inc =
                    static_cast<const cppast::cpp_include_directive &>(e);

                LOG_DBG("Processing include directive {} in file {}",
                    inc.full_path(), ctx.get_current_file().value().name());

                process_file(inc.full_path(), false, inc.include_kind());
            }
        });
}

void translation_unit_visitor::process_file(const std::string &file,
    bool register_as_current,
    std::optional<cppast::cpp_include_kind> include_kind)
{
    auto include_path = std::filesystem::path(file);

    const std::filesystem::path base_directory{ctx.config().base_directory()};

    if (include_path.is_relative()) {
        include_path = ctx.config().base_directory() / include_path;
    }

    include_path = include_path.lexically_normal();

    std::string full_file_path = include_path;
    auto f_abs = std::make_unique<common::model::source_file>();

    if (include_path.is_absolute())
        f_abs->set_absolute();

    f_abs->set_path(include_path.parent_path().string());
    f_abs->set_name(include_path.filename().string());

    if (ctx.diagram().should_include(*f_abs)) {
        if (ctx.config().relative_to) {
            const std::filesystem::path relative_to{ctx.config().relative_to()};

            include_path = std::filesystem::relative(include_path, relative_to);
        }

        auto f = std::make_unique<common::model::source_file>();

        auto parent_path_str = include_path.parent_path().string();
        if (!parent_path_str.empty())
            f->set_path(parent_path_str);
        f->set_name(include_path.filename().string());
        f->set_type(common::model::source_file_t::kHeader);

        if (register_as_current &&
            ctx.diagram().get_element(f->path() | f->name()).has_value()) {
            // This file is already in the diagram (e.g. it was added through an
            // include directive before it was visited by the parser)
            ctx.set_current_file(
                ctx.diagram().get_element(f->path() | f->name()));
            return;
        }

        if (!f->path().is_empty()) {
            // Ensure parent path directories source_files
            // are in the diagram
            common::model::filesystem_path parent_path_so_far;
            for (const auto &directory : f->path()) {
                auto dir = std::make_unique<common::model::source_file>();
                if (!parent_path_so_far.is_empty())
                    dir->set_path(parent_path_so_far);
                dir->set_name(directory);
                dir->set_type(common::model::source_file_t::kDirectory);

                if (!ctx.diagram()
                         .get_element(parent_path_so_far | directory)
                         .has_value())
                    ctx.diagram().add_file(std::move(dir));

                parent_path_so_far.append(directory);
            }
        }

        if (!register_as_current) {
            auto relationship_type =
                common::model::relationship_t::kAssociation;
            if (include_kind.has_value() &&
                include_kind.value() == cppast::cpp_include_kind::system)
                relationship_type = common::model::relationship_t::kDependency;

            ctx.get_current_file().value().add_relationship(
                common::model::relationship{relationship_type, f->alias()});

            auto fp = std::filesystem::absolute(file).lexically_normal();
            f->set_file(fp.string());
            f->set_line(0);
        }

        if (!ctx.diagram().get_element(f->path() | f->name()).has_value()) {
            ctx.set_current_file(type_safe::opt_ref(*f));
            ctx.diagram().add_file(std::move(f));
        }
    }
}
}
