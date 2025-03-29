/**
 * @file src/util/levenshtein.cc
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

#include "levenshtein.h"

namespace clanguml::util {

size_t levenshtein_distance(const std::string &left, const std::string &right)
{
    const size_t m = left.size();
    const size_t n = right.size();

    std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1, 0));

    for (size_t i = 0; i <= m; ++i)
        dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j)
        dp[0][j] = j;

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            int substitution_cost = (left[i - 1] == right[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + substitution_cost});
        }
    }

    return dp[m][n];
}

std::vector<std::string> get_approximate_matches(
    const std::vector<std::string> &collection, const std::string &pattern,
    const int max_results)
{
    std::vector<std::pair<size_t, std::string>> distance_pairs;

    for (const auto &item : collection) {
        auto distance = levenshtein_distance(item, pattern);
        distance_pairs.emplace_back(distance, item);
    }

    std::sort(distance_pairs.begin(), distance_pairs.end(),
        [](const auto &a, const auto &b) { return a.first < b.first; });

    std::vector<std::string> result;
    for (size_t i = 0; i < std::min<size_t>(distance_pairs.size(), max_results);
         ++i) {
        result.push_back(distance_pairs[i].second);
    }

    return result;
}

} // namespace clanguml::util