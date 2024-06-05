/**
 * @file src/sequence_diagram/model/message.h
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

#include "common/model/enums.h"
#include "participant.h"

#include <string>
#include <vector>

namespace clanguml::sequence_diagram::model {

/**
 * @brief Model of a sequence diagram message.
 */
class message : public common::model::diagram_element {
public:
    message() = default;

    /**
     * @brief Constructor
     *
     * @param type Message type
     * @param from Id of originating sequence
     */
    message(common::model::message_t type, eid_t from);

    /**
     * @brief Equality operator
     *
     * @param other Compare this to other message
     * @return True, if 2 messages are equal
     */
    bool operator==(const message &other) const noexcept;

    /**
     * @brief Set message type
     *
     * @param t Message type
     */
    void set_type(common::model::message_t t);

    /**
     * @brief Get message type
     *
     * @return Message type
     */
    common::model::message_t type() const;

    /**
     * @brief Set the id of message source participant
     *
     * @param f Id of the participant from which message originates
     */
    void set_from(eid_t f);

    /**
     * @brief Get the id of source of message
     *
     * @return
     */
    eid_t from() const;

    /**
     * @brief Set the id of the message target
     *
     * @param t Id of the message target
     */
    void set_to(eid_t t);

    /**
     * @brief Get the id of the message target
     *
     * @return Id of the message target
     */
    eid_t to() const;

    /**
     * @brief Set the message label
     *
     * @param name Message label
     */
    void set_message_name(std::string name);

    /**
     * @brief Get the message label
     *
     * @return Message label
     */
    const std::string &message_name() const;

    /**
     * @brief Set the return message type label
     *
     * @param t Message return type label
     */
    void set_return_type(std::string t);

    /**
     * @brief Get the return message type label
     *
     * @return Message return type label
     */
    const std::string &return_type() const;

    const std::optional<std::string> &comment() const;

    void set_comment(std::string c);

    void set_comment(const std::optional<std::string> &c);

    /**
     * @brief Set message scope
     *
     * Message scope currently means whether the message was called from
     * regular statement, or a statement embedded in a statement block condition
     *
     * @param scope Message scope
     */
    void set_message_scope(common::model::message_scope_t scope);

    /**
     * @brief Get message scope
     *
     * @return Message scope
     */
    common::model::message_scope_t message_scope() const;

    /**
     * @brief Set condition text for block statements (e.g. if(<THIS TEXT>))
     *
     * @param condition_text Condition text
     */
    void condition_text(const std::string &condition_text);

    /**
     * @brief Get condition text
     *
     * @return Block statement condition text
     */
    std::optional<std::string> condition_text() const;

    bool in_static_declaration_context() const;

    void in_static_declaration_context(bool v);

private:
    common::model::message_t type_{common::model::message_t::kNone};

    eid_t from_{};

    eid_t to_{};

    common::model::message_scope_t scope_{
        common::model::message_scope_t::kNormal};

    // This is only for better verbose messages, we cannot rely on this
    // always
    std::string message_name_{};

    std::string return_type_{};

    std::optional<std::string> condition_text_;

    std::optional<std::string> comment_;

    bool in_static_declaration_context_{false};
};

} // namespace clanguml::sequence_diagram::model
