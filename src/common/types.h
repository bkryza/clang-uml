/**
 * src/common/types.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

#include <cassert>
#include <cstdint>
#include <optional>
#include <regex>
#include <unordered_set>
#include <variant>
#include <vector>

#include "model/namespace.h"

namespace clanguml::common {

using id_t = int64_t;

enum class generator_type_t { plantuml, json };

std::string to_string(const std::string &s);

template <typename T> class optional_ref {
public:
    using optional_type = T;

    optional_ref() = default;

    optional_ref(T *value)
        : value_{value}
    {
    }

    optional_ref(const T *value)
        : value_{value}
    {
    }

    optional_ref(T &value)
        : value_{&value}
    {
    }

    optional_ref(const T &value)
        : value_{&value}
    {
    }

    optional_ref(optional_ref &right)
        : value_{right.get()}
    {
    }

    template <typename V,
        typename = std::enable_if<
            std::is_base_of_v<optional_type, typename V::optional_type> ||
            std::is_same_v<optional_type, typename V::optional_type>>>
    optional_ref(const V &t)
        : value_{t.get()}
    {
    }

    template <typename V,
        typename = std::enable_if<
            std::is_base_of_v<optional_type, typename V::optional_type> ||
            std::is_same_v<optional_type, typename V::optional_type>>>
    optional_ref(V &&t)
        : value_{t.get()}
    {
        t.reset();
    }

    template <typename V,
        typename = std::enable_if<std::is_base_of_v<optional_type, V>>>
    optional_ref(const std::reference_wrapper<V> &t)
        : value_{&t.get()}
    {
    }

    optional_ref &operator=(const optional_ref &right)
    {
        if (this == &right)
            return *this;

        value_ = right.value_;
        return *this;
    }

    optional_ref &operator=(optional_ref &&right) noexcept
    {
        if (this == &right)
            return *this;

        value_ = right.value_;
        right.reset();
        return *this;
    }

    bool has_value() const noexcept { return value_ != nullptr; }

    operator bool() const noexcept { return has_value(); }

    const T &value() const
    {
        assert(value_ != nullptr);
        return *value_;
    }

    T &value()
    {
        assert(value_ != nullptr);
        return *value_;
    }

    T &operator*()
    {
        assert(value_ != nullptr);
        return *value_;
    }

    const T &operator*() const
    {
        assert(value_ != nullptr);
        return *value_;
    }

    void reset() { value_ = nullptr; }

    T *get() const { return value_; }

private:
    T *value_{nullptr};
};

template <typename T> using opt_ref = optional_ref<T>;

template <typename T>
using reference_vector = std::vector<std::reference_wrapper<T>>;

template <typename T>
using reference_set = std::unordered_set<std::reference_wrapper<T>>;

/**
 * @brief Wrapper around std::regex, which contains original pattern
 */
struct regex {
    regex(std::regex r, std::string p)
        : regexp{std::move(r)}
        , pattern{std::move(p)}
    {
    }

    [[nodiscard]] bool operator==(const std::string &v) const
    {
        return std::regex_match(v, regexp);
    }

    std::regex regexp;
    std::string pattern;
};

template <typename T> struct or_regex {
    or_regex() = default;

    or_regex(T v)
        : value_{std::move(v)}
    {
    }

    or_regex(std::regex r, std::string p)
        : value_{regex{std::move(r), std::move(p)}}
    {
    }

    or_regex &operator=(const T &v)
    {
        value_ = v;
        return *this;
    }

    or_regex &operator=(const regex &v)
    {
        value_ = v;
        return *this;
    }

    [[nodiscard]] bool operator==(const T &v) const
    {
        if (std::holds_alternative<regex>(value_))
            return std::regex_match(v, std::get<regex>(value_).regexp);

        return std::get<T>(value_) == v;
    }

    template <typename Ret> std::optional<Ret> get() const
    {
        if (!std::holds_alternative<Ret>(value_))
            return std::nullopt;

        return std::get<Ret>(value_);
    }

    std::string to_string() const
    {
        if (std::holds_alternative<regex>(value_))
            return std::get<regex>(value_).pattern;

        return clanguml::common::to_string(std::get<T>(value_));
    }

    const std::variant<T, regex> &value() const { return value_; }

private:
    std::variant<T, regex> value_;
};

using string_or_regex = or_regex<std::string>;

std::string to_string(const string_or_regex &sr);

using namespace_or_regex = common::or_regex<common::model::namespace_>;

struct path_or_regex : public or_regex<std::filesystem::path> { };

} // namespace clanguml::common