/**
 * src/config/option.h
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

namespace clanguml {
namespace config {

template <typename T> void append_value(T &l, const T &r) { l = r; }

enum class option_inherit_mode { override, append };

template <typename T> struct option {
    option(const std::string &name_,
        option_inherit_mode im = option_inherit_mode::override)
        : name{name_}
        , value{{}}
    {
    }
    option(const std::string &name_, const T &initial_value,
        option_inherit_mode im = option_inherit_mode::override)
        : name{name_}
        , value{initial_value}
    {
    }

    void set(const T &v)
    {
        value = v;
        is_declared = true;
    }

    void override(const option<T> &o)
    {
        if (!is_declared && o.is_declared) {
            if (inheritance_mode == option_inherit_mode::append)
                append_value(value, o.value);
            else
                value = o.value;

            is_declared = true;
        }
    }

    T &operator()() { return value; }

    const T &operator()() const { return value; }

    std::string name;
    T value;
    bool is_declared{false};
    option_inherit_mode inheritance_mode;
};
}
}
