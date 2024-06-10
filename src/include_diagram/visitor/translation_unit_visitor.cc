/**
 * @file src/include_diagram/visitor/translation_unit_visitor.cc
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

#include "translation_unit_visitor.h"

#include "common/clang_utils.h"

#include <filesystem>

namespace clanguml::include_diagram::visitor {

translation_unit_visitor::translation_unit_visitor(
    clang::SourceManager & /*sm*/,
    clanguml::include_diagram::model::diagram &diagram,
    const clanguml::config::include_diagram &config)
    : diagram_{diagram}
    , config_{config}
{
}

translation_unit_visitor::include_visitor::include_visitor(
    clang::SourceManager &sm,
    clanguml::include_diagram::model::diagram &diagram,
    const clanguml::config::include_diagram &config)
    : visitor_specialization_t{sm, diagram, config}
{
}

#if LLVM_VERSION_MAJOR >= 16
void translation_unit_visitor::include_visitor::InclusionDirective(
    clang::SourceLocation hash_loc, const clang::Token & /*include_tok*/,
    clang::StringRef /*file_name*/, bool is_angled,
    clang::CharSourceRange /*filename_range*/, clang::OptionalFileEntryRef file,
    clang::StringRef /*search_path*/, clang::StringRef relative_path,
    const clang::Module * /*imported*/,
    clang::SrcMgr::CharacteristicKind file_type)
#elif LLVM_VERSION_MAJOR > 14
void translation_unit_visitor::include_visitor::InclusionDirective(
    clang::SourceLocation hash_loc, const clang::Token & /*include_tok*/,
    clang::StringRef /*file_name*/, bool is_angled,
    clang::CharSourceRange /*filename_range*/,
    clang::Optional<clang::FileEntryRef> file, clang::StringRef /*search_path*/,
    clang::StringRef relative_path, const clang::Module * /*imported*/,
    clang::SrcMgr::CharacteristicKind file_type)
#else
void translation_unit_visitor::include_visitor::InclusionDirective(
    clang::SourceLocation hash_loc, const clang::Token & /*include_tok*/,
    clang::StringRef /*file_name*/, bool is_angled,
    clang::CharSourceRange /*filename_range*/, const clang::FileEntry *file,
    clang::StringRef /*search_path*/, clang::StringRef relative_path,
    const clang::Module * /*imported*/,
    clang::SrcMgr::CharacteristicKind file_type)
#endif
{
    using common::model::relationship;
    using common::model::source_file;
    using common::model::source_file_t;

    auto current_file =
        std::filesystem::path{source_manager().getFilename(hash_loc).str()};
    current_file = std::filesystem::absolute(current_file);
    current_file = current_file.lexically_normal();

    auto current_file_id = process_source_file(current_file);
    if (!current_file_id)
        return;

    assert(diagram().get(current_file_id.value()));

#if LLVM_VERSION_MAJOR > 14
    if (!file.has_value())
        return;
    auto include_path = std::filesystem::path(file->getDir().getName().str());
#else
    if (file == nullptr)
        return;
    auto include_path = std::filesystem::path(file->getDir()->getName().str());
#endif
    include_path = include_path / file->getName().str();
    include_path = include_path.lexically_normal();

    LOG_DBG("Processing include directive {} in file {}", include_path.string(),
        current_file.string());

    auto relative_include_path =
        std::filesystem::relative(include_path, config().root_directory());

    if (diagram().should_include(source_file{include_path})) {
        process_internal_header(include_path,
            file_type != clang::SrcMgr::CharacteristicKind::C_User,
            current_file_id.value());
    }
    else if (config().generate_system_headers() && is_angled) {
        process_external_system_header(
            relative_path.str(), current_file_id.value());
    }
    else {
        LOG_DBG("Skipping include directive to file {}", include_path.string());
    }
}

void translation_unit_visitor::include_visitor::process_internal_header(
    const std::filesystem::path &include_path, bool is_system,
    const eid_t current_file_id)
{
    // Make the path relative with respect to relative_to config option
    auto relative_include_path =
        std::filesystem::relative(include_path, config().root_directory());

    // Check if this source file is already registered in the diagram,
    // if not add it
    auto diagram_path =
        common::model::source_file{relative_include_path}.full_path();
    if (!diagram().get_element(diagram_path.to_string()).has_value()) {
        diagram().add_file(std::make_unique<common::model::source_file>(
            diagram_path.to_string()));
    }

    auto &include_file = diagram().get_element(diagram_path).value();

    include_file.set_type(common::model::source_file_t::kHeader);
    include_file.set_file(
        std::filesystem::absolute(include_path).lexically_normal().string());
    include_file.set_line(0);
    include_file.set_system_header(is_system);

    // Add relationship from the currently parsed source file to this
    // include file
    const auto relationship_type = is_system
        ? common::model::relationship_t::kDependency
        : common::model::relationship_t::kAssociation;

    if (diagram().get(current_file_id)) {
        diagram()
            .get(current_file_id)
            .value()
            .add_relationship(common::model::relationship{relationship_type,
                include_file.id(), common::model::access_t::kNone});
    }
}

void translation_unit_visitor::include_visitor::process_external_system_header(
    const std::filesystem::path &include_path, const eid_t current_file_id)
{
    const auto file_name = include_path.filename();
    const auto file_name_str = file_name.string();

    auto f = std::make_unique<common::model::source_file>();
    f->set_name(include_path.string());
    f->set_type(common::model::source_file_t::kHeader);
    f->set_id(common::to_id(include_path));
    f->set_system_header(true);

    const auto f_id = f->id();

    diagram().add_file(std::move(f));

    if (diagram().get(current_file_id)) {
        diagram()
            .get(current_file_id)
            .value()
            .add_relationship(common::model::relationship{
                common::model::relationship_t::kDependency, f_id,
                common::model::access_t::kNone});
    }
}

std::optional<eid_t>
translation_unit_visitor::include_visitor::process_source_file(
    const std::filesystem::path &file)
{
    using common::model::relationship;
    using common::model::source_file;
    using common::model::source_file_t;

    auto file_path = std::filesystem::path{file};

    // Make sure the file_path is absolute with respect to the
    // filesystem, and in normal form
    if (file_path.is_relative()) {
        file_path = config().base_directory() / file_path;
    }

    file_path = file_path.lexically_normal();

    if (diagram().should_include(source_file{file_path})) {
        LOG_DBG("Processing source file {}", file.string());

        // Relativize the path with respect to effective root directory
        auto relative_file_path =
            std::filesystem::relative(file_path, config().root_directory());

        [[maybe_unused]] const auto relative_file_path_str =
            relative_file_path.string();

        // Check if this source file is already registered in the diagram,
        // if not add it
        auto diagram_path = source_file{relative_file_path}.full_path();
        if (!diagram().get_element(diagram_path).has_value()) {
            diagram().add_file(
                std::make_unique<source_file>(relative_file_path));
        }

        auto &source_file = diagram().get_element(diagram_path).value();

        const std::string implementation_suffix_prefix{".c"};
        if (file_path.has_extension() &&
            util::starts_with(
                file_path.extension().string(), implementation_suffix_prefix)) {
            source_file.set_type(source_file_t::kImplementation);
        }
        else
            source_file.set_type(source_file_t::kHeader);

        source_file.set_file(std::filesystem::absolute(file.string())
                                 .lexically_normal()
                                 .string());

        if (util::is_relative_to(file_path, config().root_directory())) {
            source_file.set_file_relative(util::path_to_url(
                relative(source_file.file(), config().root_directory())
                    .string()));
        }
        else {
            source_file.set_file_relative("");
        }

        source_file.set_line(0);

        return source_file.id();
    }

    return {};
}

clanguml::include_diagram::model::diagram &translation_unit_visitor::diagram()
{
    return diagram_;
}

const clanguml::config::include_diagram &
translation_unit_visitor::config() const
{
    return config_;
}

void translation_unit_visitor::finalize() { }

} // namespace clanguml::include_diagram::visitor
