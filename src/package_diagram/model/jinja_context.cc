/**
 * @file src/package_diagram/model/jinja_context.cc
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

#include "jinja_context.h"

namespace clanguml::common::jinja {

using namespace clanguml::common::model;

void to_json(
    inja::json &ctx, const jinja_context<package_diagram::model::diagram> &d)
{
    ctx["name"] = d.get().name();
    ctx["type"] = "package";

    inja::json::array_t elements{};

    d.get().view<common::model::package>().for_each([&](auto &&e) mutable {
        elements.emplace_back(jinja_context<element>(e));
    });

    ctx["elements"] = elements;
}

} // namespace clanguml::common::jinja