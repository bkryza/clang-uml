/**
 * @file src/common/types.h
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

#include <cassert>
#include <cstdint>
#include <optional>
#include <regex>
#include <unordered_set>
#include <variant>
#include <vector>

#include "model/namespace.h"

namespace clanguml::common {

/**
 * @brief Universal class for representing all kinds of Id's in the diagram
 *        model.
 *
 * This class provides a convenient way of representing id's in the diagram
 * model. The main problem it solves is that it allows to store both
 * Clang AST ID's (obtained using getID() method), which are local to a single
 * translation unit and have a type int64_t as well as global (across
 * multiple translation units) id's used by clang-uml in the intermediate
 * model (which are uint64_t).
 *
 * The class is aware which kind of value it holds and tries to ensure that
 * mistakes such as using AST local ID in place where global id is needed
 * do not happen.
 */
class eid_t {
public:
    using type = uint64_t;

    eid_t();

    explicit eid_t(int64_t id);

    explicit eid_t(type id);

    eid_t(const eid_t &) = default;
    eid_t(eid_t &&) noexcept = default;
    eid_t &operator=(const eid_t &) = default;
    eid_t &operator=(eid_t &&) noexcept = default;

    eid_t &operator=(int64_t ast_id);

    ~eid_t() = default;

    bool is_global() const;

    friend bool operator==(const eid_t &lhs, const eid_t &rhs);
    friend bool operator==(const eid_t &lhs, const uint64_t &v);
    friend bool operator!=(const eid_t &lhs, const uint64_t &v);
    friend bool operator!=(const eid_t &lhs, const eid_t &rhs);
    friend bool operator<(const eid_t &lhs, const eid_t &rhs);

    type value() const;

    int64_t ast_local_value() const;

private:
    type value_;
    bool is_global_;
};

/**
 * Type of output diagram format generator.
 */
enum class generator_type_t {
    plantuml, /*!< Diagrams will be generated in PlantUML format */
    json,     /*!< Diagrams will be generated in JSON format */
    mermaid   /*!< Diagrams will be generated in MermaidJS format */
};

std::string to_string(const std::string &s);

std::string to_string(generator_type_t type);

/**
 * @brief Simple optional reference type.
 *
 * This class template provides a convenient way around the std::optional
 * limitation of not allowing references. This is useful for storing
 * references to diagram elements in various `views`, and writing methods
 * which return can return an empty value if the diagram does not contain
 * something.
 *
 * This is not an owning type - it will not accept an rvalue - the actual
 * value must be stored somewhere else as lvalue or some kind of smart pointer.
 *
 * @note Probably unsafe - do not use at home.
 *
 * @tparam T Type of reference
 */
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
    /**
     * @brief Constructor
     *
     * @param r Parsed regular expression
     * @param p Raw regular expression pattern used for regenerating config and
     *          debugging
     */
    regex(std::regex r, std::string p)
        : regexp{std::move(r)}
        , pattern{std::move(p)}
    {
    }

    /**
     * @brief Regular expression match operator
     * @param v Value to match against internal std::regex
     * @return True, if the argument matches the regular expression
     */
    [[nodiscard]] bool operator%=(const std::string &v) const
    {
        return std::regex_match(v, regexp);
    }

    std::regex regexp;   /*!< Parsed regular expression */
    std::string pattern; /*!< Original regular expression pattern */
};

/**
 * @brief Convenience class for configuration options with regex support
 *
 * This template class provides a convenient way of handling configuraiton
 * options, which can be either some basic type like std::string or regex.
 *
 * @tparam T Type of alternative to regex (e.g. std::string)
 */
template <typename T> struct or_regex {
    or_regex() = default;

    /**
     * @brief Constructor from alternative type
     */
    or_regex(T v)
        : value_{std::move(v)}
    {
    }

    /**
     * @brief Constructor from regex
     *
     * @param r Parsed regular expression
     * @param p Raw regular expression pattern used for regenerating config and
     *          debugging
     */
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

template <> class fmt::formatter<clanguml::common::eid_t> {
public:
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
    template <typename Context>
    constexpr auto format(clanguml::common::eid_t const &id, Context &ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", id.value());
    }
};
