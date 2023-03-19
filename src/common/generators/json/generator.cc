/**
 * src/common/generators/json/generator.cc
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

#include "generator.h"

namespace clanguml::common::model {
using nlohmann::json;

void to_json(nlohmann::json &j, const source_location &sl)
{
    j = json{{"file", sl.file_relative()}, {"line", sl.line()}};
}

void to_json(nlohmann::json &j, const element &c)
{
    j = json{{"id", std::to_string(c.id())}, {"name", c.name()},
        {"namespace", c.get_namespace().to_string()}, {"type", c.type_name()},
        {"display_name", c.full_name(false)}};

    if (const auto &comment = c.comment(); comment)
        j["comment"] = comment.value();

    if (!c.file().empty()) {
        j["source_location"] =
            dynamic_cast<const common::model::source_location &>(c);
    }
}

void to_json(nlohmann::json &j, const template_parameter &c)
{
    j["kind"] = to_string(c.kind());
    if (c.type())
        j["type"] = c.type().value();
    if (c.name())
        j["name"] = c.name().value();
    if (c.default_value())
        j["default"] = c.default_value().value();
    j["is_variadic"] = c.is_variadic();
}

void to_json(nlohmann::json &j, const relationship &c)
{
    j["type"] = to_string(c.type());
    j["destination"] = std::to_string(c.destination());
    if (!c.multiplicity_source().empty())
        j["multiplicity_source"] = c.multiplicity_source();
    if (!c.multiplicity_destination().empty())
        j["multiplicity_destination"] = c.multiplicity_destination();
    if (c.access() != access_t::kNone)
        j["access"] = to_string(c.access());
    if (!c.label().empty())
        j["label"] = c.label();
    if (const auto &comment = c.comment(); comment)
        j["comment"] = comment.value();
}

} // namespace clanguml::common::model
