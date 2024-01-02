/**
 * @file src/common/model/source_location.h
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
#include <utility>

namespace clanguml::common::model {

/**
 * @brief Base class of all diagram elements that have source location.
 *
 * @embed{source_location_hierarchy_class.svg}
 */
class source_location {
public:
    source_location() = default;

    source_location(std::string f, unsigned int l)
        : file_{std::move(f)}
        , line_{l}
    {
    }

    /**
     * Return absolute source file path.
     *
     * @return Absolute file path.
     */
    const std::string &file() const { return file_; }

    /**
     * Set absolute file path.
     *
     * @param file Absolute file path.
     */
    void set_file(const std::string &file) { file_ = file; }

    /**
     * Return source file path relative to `relative_to` config option.
     *
     * @return Relative file path.
     */
    const std::string &file_relative() const { return file_relative_; }

    /**
     * Set relative file path.
     *
     * @param file Relative file path.
     */
    void set_file_relative(const std::string &file) { file_relative_ = file; }

    /**
     * Get the translation unit, from which this source location was visited.
     *
     * @return Path to the translation unit.
     */
    const std::string &translation_unit() const { return translation_unit_; }

    /**
     * Set the path to translation unit, from which this source location was
     * visited.
     *
     * @param translation_unit Path to the translation unit.
     */
    void set_translation_unit(const std::string &translation_unit)
    {
        translation_unit_ = translation_unit;
    }

    /**
     * Get the source location line number.
     *
     * @return Line number.
     */
    unsigned int line() const { return line_; }

    /**
     * Set the source location line number.
     *
     * @param line Line number.
     */
    void set_line(const unsigned line) { line_ = line; }

    /**
     * Get the source location column number.
     *
     * @return Column number.
     */
    unsigned int column() const { return column_; }

    /**
     * Set the source location column number.
     *
     * @param line Column number.
     */
    void set_column(const unsigned column) { column_ = column; }

    /**
     * Get the source location id.
     *
     * The location id is equivalent to Clang's SourceLocation::getHashValue()
     *
     * @return Location id.
     */
    unsigned int location_id() const { return hash_; }

    /**
     * Set the source location id.
     *
     * The location id is equivalent to Clang's SourceLocation::getHashValue()
     *
     * @param h Location id.
     */
    void set_location_id(unsigned int h) { hash_ = h; }

private:
    std::string file_;
    std::string file_relative_;
    std::string translation_unit_;
    unsigned int line_{0};
    unsigned int column_{0};
    unsigned int hash_{0};
};
} // namespace clanguml::common::model