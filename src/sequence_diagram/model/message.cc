/**
 * src/sequence_diagram/model/message.cc
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

#include "message.h"

namespace clanguml::sequence_diagram::model {

message::message(
    common::model::message_t type, common::model::diagram_element::id_t from)
    : type_{type}
    , from_{from}
{
}

void message::set_type(common::model::message_t t) { type_ = t; }

common::model::message_t message::type() const { return type_; }

void message::set_from(common::model::diagram_element::id_t f) { from_ = f; }

common::model::diagram_element::id_t message::from() const { return from_; }

void message::set_to(common::model::diagram_element::id_t t) { to_ = t; }

common::model::diagram_element::id_t message::to() const { return to_; }

void message::set_message_name(std::string name)
{
    message_name_ = std::move(name);
}

const std::string &message::message_name() const { return message_name_; }

void message::set_return_type(std::string t) { return_type_ = std::move(t); }

const std::string &message::return_type() const { return return_type_; }

}
