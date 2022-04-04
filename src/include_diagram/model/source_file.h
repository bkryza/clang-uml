/**
 * src/include_diagram/model/source_file.h
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
#pragma once

#include "common/model/diagram_element.h"
#include "common/model/nested_trait.h"
#include "common/model/path.h"
#include "common/model/stylable_element.h"
#include "util/util.h"

#include <spdlog/spdlog.h>
#include <type_safe/optional_ref.hpp>

#include <set>
#include <string>
#include <vector>

namespace clanguml::include_diagram::model {

enum class source_file_type { kDirectory, kHeader, kImplementation };

struct fs_path_sep {
    static constexpr std::string_view value = "/";
};

using filesystem_path = common::model::path<fs_path_sep>;

class source_file
    : public common::model::diagram_element,
      public common::model::stylable_element,
      public common::model::nested_trait<common::model::diagram_element,
          filesystem_path> {
public:
    source_file() = default;

    void set_path(const filesystem_path &p) { path_ = p; }

    source_file(const source_file &) = delete;
    source_file(source_file &&) = default;
    source_file &operator=(const source_file &) = delete;
    source_file &operator=(source_file &&) = delete;

    const filesystem_path &path() const { return path_; }

    std::string full_name(bool relative) const override
    {
        return (path_ | name()).to_string();
    }

    void add_file(std::unique_ptr<include_diagram::model::source_file> &&f)
    {
        LOG_DBG("Adding source file: {}, {}", f->name(), f->full_name(true));

        add_element(f->path(), std::move(f));
    }

private:
    filesystem_path path_;
};
}

namespace std {

template <> struct hash<clanguml::include_diagram::model::filesystem_path> {
    std::size_t operator()(
        const clanguml::include_diagram::model::filesystem_path &key) const
    {
        using clanguml::common::model::path;

        std::size_t seed = key.size();
        for (const auto &ns : key) {
            seed ^= std::hash<std::string>{}(ns) + 0x6a3712b5 + (seed << 6) +
                (seed >> 2);
        }

        return seed;
    }
};

}
