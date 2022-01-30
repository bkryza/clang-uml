/**
 * src/common/model/relationship.h
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

#include "common/model/decorated_element.h"
#include "common/model/stylable_element.h"

#include <string>

namespace clanguml::common::model {

std::string to_string(relationship_t r);

class relationship : public common::model::decorated_element,
                     public common::model::stylable_element {
public:
    relationship(relationship_t type, const std::string &destination,
        scope_t scope = scope_t::kNone, const std::string &label = "",
        const std::string &multiplicity_source = "",
        const std::string &multiplicity_destination = "");

    virtual ~relationship() = default;

    void set_type(relationship_t type) noexcept;
    relationship_t type() const noexcept;

    void set_destination(const std::string &destination);
    std::string destination() const;

    void set_multiplicity_source(const std::string &multiplicity_source);
    std::string multiplicity_source() const;

    void set_multiplicity_destination(
        const std::string &multiplicity_destination);
    std::string multiplicity_destination() const;

    void set_label(const std::string &label);
    std::string label() const;

    void set_scope(scope_t scope) noexcept;
    scope_t scope() const noexcept;

    friend bool operator==(const relationship &l, const relationship &r);

private:
    relationship_t type_{relationship_t::kAssociation};
    std::string destination_;
    std::string multiplicity_source_;
    std::string multiplicity_destination_;
    std::string label_;
    scope_t scope_{scope_t::kNone};
};
}
