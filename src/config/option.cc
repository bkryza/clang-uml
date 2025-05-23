/**
 * @file src/config/option.cc
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

#include "option.h"

namespace clanguml::config {

template <> void append_value(inja::json &l, const inja::json &r)
{
    if (r.is_object()) {
        if (l.is_null()) {
            l = r;
        }
        else {
            inja::json merged = r;
            merged.merge_patch(l);
            l = std::move(merged);
        }
    }
}

} // namespace clanguml::config