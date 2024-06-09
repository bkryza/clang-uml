/**
 * @file src/include_diagram/model/diagram.cc
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

#include "diagram.h"

#include "util/error.h"
#include "util/util.h"

namespace clanguml::include_diagram::model {

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kInclude;
}

common::optional_ref<common::model::diagram_element> diagram::get(
    const std::string &full_name) const
{
    return find<source_file>(full_name);
}

common::optional_ref<common::model::diagram_element> diagram::get(
    const eid_t id) const
{
    return find<source_file>(id);
}

void diagram::add_file(std::unique_ptr<common::model::source_file> &&f)
{
    // Don't add the same file more than once
    if (find<source_file>(f->id()))
        return;

    LOG_DBG("Adding source file: {}, {}", f->name(), f->fs_path().string());

    auto &ff = *f;

    assert(!ff.name().empty());
    assert(ff.id().value() != 0);

    element_view<source_file>::add(ff);

    auto p = ff.path();

    if (!f->path().is_empty()) {
        // If the parent path is not empty, ensure relative parent directories
        // of this source_file are in the diagram
        common::model::filesystem_path parent_path_so_far{
            common::model::path_type::kFilesystem};
        for (const auto &directory : f->path()) {
            auto source_file_path = parent_path_so_far | directory;
            if (parent_path_so_far.is_empty())
                source_file_path = {directory};

            auto dir = std::make_unique<common::model::source_file>(
                std::filesystem::path{source_file_path.to_string()});
            dir->set_type(common::model::source_file_t::kDirectory);

            assert(!dir->name().empty());

            if (!get_element(source_file_path).has_value()) {
                add_file(std::move(dir));

                LOG_DBG("Added directory '{}' at path '{}'", directory,
                    parent_path_so_far.to_string());
            }

            parent_path_so_far.append(directory);
        }
    }

    assert(p.type() == common::model::path_type::kFilesystem);

    add_element(p, std::move(f));
}

const common::reference_vector<common::model::source_file> &
diagram::files() const
{
    return element_view<source_file>::view();
}

common::optional_ref<clanguml::common::model::diagram_element>
diagram::get_with_namespace(
    const std::string &name, const common::model::namespace_ &ns) const
{
    // Convert to preferred OS path
    std::filesystem::path namePath{name};
    auto namePreferred = namePath.make_preferred().string();

    auto element_opt = get(namePreferred);

    if (!element_opt) {
        // If no element matches, try to prepend the 'using_namespace'
        // value to the element and search again
        auto fully_qualified_name = ns | namePreferred;
        element_opt = get(fully_qualified_name.to_string());
    }

    return element_opt;
}

inja::json diagram::context() const
{
    inja::json ctx;
    ctx["name"] = name();
    ctx["type"] = "include";

    inja::json::array_t elements{};

    // Add files and directories
    for (const auto &f : files()) {
        elements.emplace_back(f.get().context());
    }

    ctx["elements"] = elements;

    return ctx;
}

bool diagram::is_empty() const { return element_view<source_file>::is_empty(); }

} // namespace clanguml::include_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<include_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kInclude;
}
}
