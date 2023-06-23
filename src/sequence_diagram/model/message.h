/**
 * @file src/sequence_diagram/model/message.h
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
#pragma once

#include "common/model/enums.h"
#include "participant.h"

#include <string>
#include <vector>

namespace clanguml::sequence_diagram::model {

class message : public common::model::diagram_element {
public:
    message() = default;

    message(common::model::message_t type,
        common::model::diagram_element::id_t from);

    void set_type(common::model::message_t t);
    common::model::message_t type() const;

    void set_from(common::model::diagram_element::id_t f);
    common::model::diagram_element::id_t from() const;

    void set_to(common::model::diagram_element::id_t t);
    common::model::diagram_element::id_t to() const;

    void set_message_name(std::string name);
    const std::string &message_name() const;

    void set_return_type(std::string t);
    const std::string &return_type() const;

    void set_message_scope(common::model::message_scope_t scope);
    common::model::message_scope_t message_scope() const;

private:
    common::model::message_t type_{common::model::message_t::kNone};

    common::model::diagram_element::id_t from_{};

    common::model::diagram_element::id_t to_{};

    common::model::message_scope_t scope_{
        common::model::message_scope_t::kNormal};

    // This is only for better verbose messages, we cannot rely on this
    // always
    std::string message_name_{};

    std::string return_type_{};
};

} // namespace clanguml::sequence_diagram::model
