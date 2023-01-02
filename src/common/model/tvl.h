/**
 * src/class_diagram/model/tvl.h
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

#include <optional>
#include <string>

namespace clanguml::common::model::tvl {

using value_t = std::optional<bool>;

inline bool is_true(const value_t &v) { return v.has_value() && v.value(); }

inline bool is_false(const value_t &v) { return v.has_value() && !v.value(); }

inline bool is_undefined(const value_t &v) { return !v.has_value(); }

template <typename InputIterator, typename Predicate>
inline value_t all_of(InputIterator first, InputIterator last, Predicate pred)
{
    value_t res{};

    for (InputIterator it = first; it != last; it++) {
        value_t m = pred(*it);
        if (m.has_value()) {
            if (m.value()) {
                res = true;
            }
            else {
                res = false;
                break;
            }
        }
    }

    return res;
}

template <typename InputIterator, typename Predicate>
inline value_t any_of(InputIterator first, InputIterator last, Predicate pred)
{
    value_t res{};

    for (InputIterator it = first; it != last; it++) {
        value_t m = pred(*it);
        if (m.has_value()) {
            if (m.value()) {
                res = true;
                break;
            }
            res = false;
        }
    }

    return res;
}
} // namespace clanguml::common::model::tvl