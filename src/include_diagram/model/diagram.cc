/**
 * src/include_diagram/model/diagram.cc
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

#include "diagram.h"

#include "util/error.h"
#include "util/util.h"

namespace clanguml::include_diagram::model {

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kPackage;
}

type_safe::optional_ref<const common::model::diagram_element> diagram::get(
    const std::string &full_name) const
{
    return get_file(full_name);
}

void diagram::add_file(std::unique_ptr<common::model::source_file> &&f)
{
    LOG_DBG("Adding source file: {}, {}", f->name(), f->full_name(true));

    files_.emplace_back(*f);

    auto p = f->path();

    if (!f->path().is_empty()) {
        // If the parent path is not empty, ensure relative parent directories
        // of this source_file are in the diagram
        common::model::filesystem_path parent_path_so_far;
        for (const auto &directory : f->path()) {
            auto dir = std::make_unique<common::model::source_file>();
            if (!parent_path_so_far.is_empty())
                dir->set_path(parent_path_so_far);
            dir->set_name(directory);
            dir->set_type(common::model::source_file_t::kDirectory);

            if (!get_element(parent_path_so_far | directory).has_value())
                add_file(std::move(dir));

            parent_path_so_far.append(directory);
        }
    }

    add_element(p, std::move(f));
}

type_safe::optional_ref<const common::model::source_file> diagram::get_file(
    const std::string &name) const
{
    for (const auto &p : files_) {
        if (p.get().full_name(false) == name) {
            return {p};
        }
    }

    return type_safe::nullopt;
}

std::string diagram::to_alias(const std::string &full_name) const
{
    LOG_DBG("Looking for alias for {}", full_name);

    auto path = common::model::filesystem_path{full_name};

    if (path.is_empty())
        throw error::uml_alias_missing(
            fmt::format("Missing alias for '{}'", path.to_string()));

    auto source_file = get_element<common::model::source_file>(path);

    if (!source_file)
        throw error::uml_alias_missing(
            fmt::format("Missing alias for '{}'", path.to_string()));

    return source_file.value().alias();
}

}
