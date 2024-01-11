/**
 * @file src/config/option.h
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

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace clanguml {
namespace config {

template <typename T> void append_value(T &l, const T &r) { l = r; }

template <typename T>
void append_value(std::vector<T> &l, const std::vector<T> &r)
{
    l.insert(std::end(l), r.begin(), r.end());
}

template <typename K, typename V>
void append_value(std::map<K, V> &l, const std::map<K, V> &r)
{
    l.insert(r.begin(), r.end());
}

/**
 * Possible option inheritance methods from top level to diagram level.
 */
enum class option_inherit_mode {
    kOverride, /*!< Override entire options */
    kAppend    /*!< Append to list options */
};

struct option_with_alt_names_tag { };

/**
 * @brief Generic configuration option type
 *
 * This class template represents a single configuration option, which can
 * be either a simple type such as bool or std::string or can be a list
 * or dictionary.
 *
 * If the option is constructed only from default value, it's `is_declared`
 * member is false, so we can deduce whether user provided the option or not.
 *
 * For each option type, there has to be defined a YAML decoder and emitter.
 *
 * @tparam T The type of the configuration option
 */
template <typename T> struct option {
    option(std::string name_,
        option_inherit_mode im = option_inherit_mode::kOverride)
        : name{std::move(name_)}
        , value{}
        , inheritance_mode{im}
    {
    }
    option(std::string name_, T initial_value,
        option_inherit_mode im = option_inherit_mode::kOverride)
        : name{std::move(name_)}
        , value{std::move(initial_value)}
        , has_value{true}
        , inheritance_mode{im}
    {
    }
    option(option_with_alt_names_tag /*unused*/, std::string name_,
        std::vector<std::string> alternate_names_,
        option_inherit_mode im = option_inherit_mode::kOverride)
        : name{std::move(name_)}
        , alternate_names{std::move(alternate_names_)}
        , value{}
        , inheritance_mode{im}
    {
    }

    /**
     * @brief Set the option value
     *
     * @param v Option value
     */
    void set(const T &v)
    {
        value = v;
        is_declared = true;
        has_value = true;
    }

    /**
     * @brief Override option value
     *
     * This method overrides the option depending on it's inheritance type.
     *
     * @param o New option value
     */
    void override(const option<T> &o)
    {
        if (o.is_declared && inheritance_mode == option_inherit_mode::kAppend) {
            append_value(value, o.value);
            is_declared = true;
            has_value = true;
        }
        else if (!is_declared && o.is_declared) {
            set(o.value);
        }
    }

    void operator()(const T &v) { set(v); }

    T &operator()() { return value; }

    const T &operator()() const { return value; }

    operator bool() const { return has_value; }

    /*! Option name, it is also the YAML key in the configuration file */
    std::string name;

    /*! Alternate option names */
    std::vector<std::string> alternate_names;

    /*! Option value */
    T value;

    /*! Whether or not the value was provided by the user or default */
    bool is_declared{false};

    /*!
     * Whether the option has value, if the option has no default value
     * and wasn't provided in the config this is set to `false`.
     */
    bool has_value{false};

    /*! The inheritance mode for this option */
    option_inherit_mode inheritance_mode;
};
} // namespace config
} // namespace clanguml
