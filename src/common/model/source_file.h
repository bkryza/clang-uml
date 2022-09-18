/**
 * src/common/model/source_file.h
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

#include "common/clang_utils.h"
#include "common/model/diagram_element.h"
#include "common/model/nested_trait.h"
#include "common/model/path.h"
#include "common/model/source_location.h"
#include "common/model/stylable_element.h"
#include "common/types.h"
#include "util/util.h"

#include <spdlog/spdlog.h>

#include <set>
#include <string>
#include <vector>

namespace clanguml::common::model {

enum class source_file_t { kDirectory, kHeader, kImplementation };

struct fs_path_sep {
    static constexpr std::string_view value = "/";
};

using filesystem_path = common::model::path<fs_path_sep>;

class source_file
    : public common::model::diagram_element,
      public common::model::stylable_element,
      public common::model::nested_trait<common::model::source_file,
          filesystem_path> {
public:
    source_file() = default;

    explicit source_file(const std::filesystem::path &p)
    {
        set_path({p.parent_path().string()});
        set_name(p.filename());
        is_absolute_ = p.is_absolute();
        set_id(common::to_id(p));
    }

    void set_path(const filesystem_path &p) { path_ = p; }

    void set_absolute() { is_absolute_ = true; }

    bool is_absolute() const { return is_absolute_; }

    void set_type(source_file_t type) { type_ = type; }

    source_file_t type() const { return type_; }

    source_file(const source_file &) = delete;
    source_file(source_file &&) = default;
    source_file &operator=(const source_file &) = delete;
    source_file &operator=(source_file &&) = delete;

    bool operator==(const source_file &right) const
    {
        return (path_ == right.path_) && (name() == right.name()) &&
            (type_ == right.type_);
    }

    const filesystem_path &path() const { return path_; }

    std::string full_name(bool /*relative*/) const override
    {
        return (path_ | name()).to_string();
    }

    auto full_path() const { return path() | name(); }

    void add_file(std::unique_ptr<source_file> &&f)
    {
        LOG_DBG("Adding source file: {}, {}", f->name(), f->full_name(true));

        add_element(f->path(), std::move(f));
    }

    std::filesystem::path fs_path(const std::filesystem::path &base = {}) const
    {
        std::filesystem::path res;

        for (const auto &path_element : path_) {
            res /= path_element;
        }

        res /= name();

        if (is_absolute_)
            res = "/" / res;
        else
            res = base / res;

        return res.lexically_normal();
    }

private:
    filesystem_path path_;
    source_file_t type_{source_file_t::kDirectory};
    bool is_absolute_{false};
};
}

namespace std {

template <> struct hash<clanguml::common::model::filesystem_path> {
    std::size_t operator()(
        const clanguml::common::model::filesystem_path &key) const
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

namespace std {
template <>
struct hash<std::reference_wrapper<clanguml::common::model::source_file>> {
    std::size_t operator()(
        const std::reference_wrapper<clanguml::common::model::source_file> &key)
        const
    {
        using clanguml::common::id_t;

        return std::hash<id_t>{}(key.get().id());
    }
};
}