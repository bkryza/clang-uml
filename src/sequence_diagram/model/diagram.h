/**
 * src/sequence_diagram/model/diagram.h
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

#include "activity.h"
#include "common/model/diagram.h"
#include "common/types.h"

#include <map>
#include <string>

namespace clanguml::sequence_diagram::model {

class diagram : public clanguml::common::model::diagram {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    common::model::diagram_t type() const override;

    common::optional_ref<common::model::diagram_element> get(
        const std::string &full_name) const override;

    common::optional_ref<common::model::diagram_element> get(
        const common::model::diagram_element::id_t id) const override;

    std::string to_alias(const std::string &full_name) const;

    bool started{false};

    std::map<std::uint_least64_t, activity> sequences;
};

}

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::sequence_diagram::model::diagram>(
    diagram_t t);
}