/**
 * src/class_diagram/visitor/translation_unit_visitor.h
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

#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

namespace clanguml::common {

using id_t = int64_t;

template <typename T>
using optional_ref = std::optional<std::reference_wrapper<const T>>;

template <typename T>
using reference_vector = std::vector<std::reference_wrapper<const T>>;

template <typename T>
using reference_set = std::unordered_set<std::reference_wrapper<const T>>;

} // namespace clang::common