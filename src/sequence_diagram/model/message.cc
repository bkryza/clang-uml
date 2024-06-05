/**
 * @file src/sequence_diagram/model/message.cc
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

#include "message.h"

namespace clanguml::sequence_diagram::model {

message::message(common::model::message_t type, eid_t from)
    : type_{type}
    , from_{from}
{
}

bool message::operator==(const message &other) const noexcept
{
    return from_ == other.from_ && to_ == other.to_ && type_ == other.type_ &&
        scope_ == other.scope_ && message_name_ == other.message_name_ &&
        return_type_ == other.return_type_ &&
        condition_text_ == other.condition_text_;
}

void message::set_type(common::model::message_t t) { type_ = t; }

common::model::message_t message::type() const { return type_; }

void message::set_from(eid_t f) { from_ = f; }

eid_t message::from() const { return from_; }

void message::set_to(eid_t t) { to_ = t; }

eid_t message::to() const { return to_; }

void message::set_message_name(std::string name)
{
    message_name_ = std::move(name);
}

const std::string &message::message_name() const { return message_name_; }

void message::set_return_type(std::string t) { return_type_ = std::move(t); }

const std::string &message::return_type() const { return return_type_; }

const std::optional<std::string> &message::comment() const { return comment_; }

void message::set_comment(std::string c)
{
    if (!c.empty())
        comment_ = std::move(c);
}

void message::set_comment(const std::optional<std::string> &c)
{
    if (c)
        set_comment(c.value());
}

void message::set_message_scope(common::model::message_scope_t scope)
{
    scope_ = scope;
}

common::model::message_scope_t message::message_scope() const { return scope_; }

void message::condition_text(const std::string &condition_text)
{
    if (condition_text.empty())
        condition_text_ = std::nullopt;
    else
        condition_text_ = condition_text;
}

std::optional<std::string> message::condition_text() const
{
    return condition_text_;
}

bool message::in_static_declaration_context() const
{
    return in_static_declaration_context_;
}

void message::in_static_declaration_context(bool v)
{
    in_static_declaration_context_ = v;
}

} // namespace clanguml::sequence_diagram::model
