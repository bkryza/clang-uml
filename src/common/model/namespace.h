/**
 * @file src/common/model/namespace.h
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

#include "path.h"

#include <optional>
#include <string>
#include <vector>

namespace clanguml::common::model {

struct ns_path_separator {
    static constexpr std::string_view value = "::";
};

using namespace_ = path;

}

namespace std {

template <> struct hash<clanguml::common::model::namespace_> {
    std::size_t operator()(const clanguml::common::model::namespace_ &key) const
    {
        using clanguml::common::model::path;

        std::size_t seed = key.size();
        for (const auto &ns : key) {
            seed ^=
                std::hash<std::string>{}(ns) + clanguml::util::hash_seed(seed);
        }

        return seed;
    }
};

} // namespace std
