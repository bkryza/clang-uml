/**
 * @file src/util/levenshtein.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include <stdexcept>

#include <algorithm>
#include <string>
#include <vector>

namespace clanguml::util {

/**
 * @brief Compute the Levenshtein distance between two strings.
 *
 * @param left First string
 * @param right Second string
 * @return Calculated Levenshtein distance
 */
size_t levenshtein_distance(const std::string &left, const std::string &right);

/**
 * @brief Find most similar strings in collection
 *
 * Returns at most three strings from `items` that are closest to `pattern`
 * based on Levenshtein distance.
 *
 * @param collection Input collection
 * @param pattern Pattern to match against items in collection
 * @return List of most similar items from collection
 */

std::vector<std::string> get_approximate_matches(
    const std::vector<std::string> &collection, const std::string &pattern,
    int max_results = 3);

} // namespace clanguml::util