/**
 * @file src/common/model/source_file.h
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

/**
 * This enum represents different kinds of files in the diagram.
 */
enum class source_file_t {
    kDirectory,     /*!< Diagram element is a directory */
    kHeader,        /*!< Diagram element is a header */
    kImplementation /*!< Diagram element is a source file (e.g. cpp) */
};

std::string to_string(source_file_t sf);

struct fs_path_sep {
#ifdef _WIN32
    static constexpr std::string_view value = "\\";
#else
    static constexpr std::string_view value = "/";
#endif
};

using filesystem_path = common::model::path;

/**
 * @brief Diagram element representing some file or directory.
 *
 * @embed{source_file_hierarchy_class.svg}
 */
class source_file
    : public common::model::diagram_element,
      public common::model::stylable_element,
      public common::model::nested_trait<common::model::source_file,
          filesystem_path> {
public:
    source_file() = default;

    explicit source_file(const std::filesystem::path &p)
    {
        auto preferred = p;
        preferred.make_preferred();
        set_path({preferred.parent_path().string(), path_type::kFilesystem});
        set_name(preferred.filename().string());
        is_absolute_ = preferred.is_absolute();
        set_id(common::to_id(preferred));
    }

    source_file(const source_file &) = delete;
    source_file(source_file &&) = default;
    source_file &operator=(const source_file &) = delete;
    source_file &operator=(source_file &&) = delete;

    bool operator==(const source_file &right) const
    {
        return (path_ == right.path_) && (name() == right.name()) &&
            (type_ == right.type_);
    }

    /**
     * Set the path to the element in the diagram.
     *
     * @param p Diagram path.
     */
    void set_path(const filesystem_path &p) { path_ = p; }

    /**
     * Is the elements path absolute?
     *
     * @return True if the elements path is absolute.
     */
    bool is_absolute() const { return is_absolute_; }

    /**
     * Set the type of the source file.
     *
     * @param type Type of the source file.
     */
    void set_type(source_file_t type) { type_ = type; }

    /**
     * Get the source file elements type.
     *
     * @return Type of the source file.
     */
    source_file_t type() const { return type_; }

    /**
     * Set whether the file is a system header
     *
     * @param is_system Whether the file is a system header
     */
    void set_system_header(bool is_system) { is_system_header_ = is_system; }

    /**
     * Is the file a system header?
     *
     * @return True, if the source file is a system header
     */
    bool is_system_header() const { return is_system_header_; }

    /**
     * Get the source file's parent path.
     *
     * @return Source file parent path.
     */
    const filesystem_path &path() const { return path_; }

    /**
     * Return the full path string, i.e. parent path and elements name.
     *
     * @return Full source file path as string.
     */
    std::string full_name(bool /*relative*/) const override
    {
        return (path_ | name()).to_string();
    }

    /**
     * Return full path, i.e. parent path and elements name.
     *
     * @return Full source file path.
     */
    auto full_path() const { return path() | name(); }

    /**
     * Convert the source file path to std::filesystem::path, relative to `base`
     *
     * @param base Base path
     * @return Filesystem path to the source file.
     */
    std::filesystem::path fs_path(const std::filesystem::path &base = {}) const
    {
        std::filesystem::path res;

        for (const auto &path_element : path_) {
            res /= path_element;
        }

        res /= name();

        if (is_absolute_)
            res = fs_path_sep::value / res;
        else
            res = base / res;

        return res.lexically_normal();
    }

    /**
     * Return inja context for this element.
     *
     * @return Inja context.
     */
    inja::json context() const override
    {
        inja::json ctx = diagram_element::context();

        std::filesystem::path fullNamePath{ctx["full_name"].get<std::string>()};
        fullNamePath.make_preferred();
        ctx["full_name"] = fullNamePath.string();

        return ctx;
    }

private:
    filesystem_path path_{path_type::kFilesystem};
    source_file_t type_{source_file_t::kDirectory};
    bool is_absolute_{false};
    bool is_system_header_{false};
};
} // namespace clanguml::common::model

namespace std {
template <>
struct hash<std::reference_wrapper<clanguml::common::model::source_file>> {
    std::size_t operator()(
        const std::reference_wrapper<clanguml::common::model::source_file> &key)
        const
    {
        return key.get().id().value();
    }
};
} // namespace std