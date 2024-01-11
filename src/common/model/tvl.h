/**
 * @file src/common/model/tvl.h
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

#include <optional>
#include <string>

/**
 * This namespace implements convenience functions to handle 3-value logic
 * operations needed for applying diagram filters.
 *
 * @see clanguml::common::model::diagram_filter
 * @see clanguml::common::model::filter_visitor
 */
namespace clanguml::common::model::tvl {

/**
 * Alias for 3-value logic values
 *
 * If optional holds nullopt, the value is undefined.
 */
using value_t = std::optional<bool>;

/**
 * Calculate 3-value logic AND value.
 *
 * @param l Left value
 * @param r Right value
 * @return Result of AND operation.
 */
inline value_t and_(const value_t &l, const value_t &r)
{
    if (!l.has_value())
        return r;

    if (!r.has_value())
        return l;

    return r.value() && l.value();
}

/**
 * Calculate 3-value logic OR value.
 *
 * @param l Left value
 * @param r Right value
 * @return Result of OR operation.
 */
inline value_t or_(const value_t &l, const value_t &r)
{
    if (!l.has_value() && !r.has_value())
        return {};

    if (l.has_value() && l.value())
        return true;

    if (r.has_value() && r.value())
        return true;

    return false;
}

/**
 * Whether the value holds true
 *
 * @param v Logical value
 * @return True, if v holds true
 */
inline bool is_true(const value_t &v) { return v.has_value() && v.value(); }

/**
 * Whether the value holds false
 *
 * @param v Logical value
 * @return True, if v holds false
 */
inline bool is_false(const value_t &v) { return v.has_value() && !v.value(); }

/**
 * Whether the value is undefined
 *
 * @param v Logical value
 * @return True, if v has no value
 */
inline bool is_undefined(const value_t &v) { return !v.has_value(); }

/**
 * 3-value logic equivalent of std::all_of
 *
 * @tparam InputIterator Iterator type
 * @tparam Predicate Predicate type
 * @param first First iterator element
 * @param last Last iterator element
 * @param pred Predicate to apply to each element
 * @return True, if all elements are true or undefined
 */
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

/**
 * 3-value logic equivalent of std::any_of
 *
 * @tparam InputIterator Iterator type
 * @tparam Predicate Predicate type
 * @param first First iterator element
 * @param last Last iterator element
 * @param pred Predicate to apply to each element
 * @return True, if at least 1 element is true
 */
template <typename InputIterator, typename Predicate>
inline value_t any_of(InputIterator first, InputIterator last, Predicate pred)
{
    value_t res{};

    for (InputIterator it = first; it != last; it++) {
        value_t m = pred(*it);
        if (m.has_value()) {
            if (m.value()) {
                res = true;
                return res;
            }
            res = false;
        }
    }

    return res;
}
} // namespace clanguml::common::model::tvl