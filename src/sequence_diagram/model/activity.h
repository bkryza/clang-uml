/**
 * @file src/sequence_diagram/model/activity.h
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

#include "message.h"
#include "participant.h"

#include <string>
#include <vector>

namespace clanguml::sequence_diagram::model {

/**
 * @brief Model of a sequence diagram activity
 */
class activity {
public:
    /**
     * @brief Constructor
     *
     * @param id Id of the participant parent for the activity
     */
    activity(eid_t id);

    /**
     * @brief Add a message call to the activity
     *
     * @param m Message model
     */
    void add_message(message m);

    /**
     * @brief Get list of messages in the activity
     *
     * @return Reference to list of messages in the activity
     */
    std::vector<message> &messages();

    /**
     * @brief Get list of messages in the activity
     *
     * @return Reference to list of messages in the activity
     */
    const std::vector<message> &messages() const;

    /**
     * @brief Get the id of activity parent participant
     *
     * @return Id of activity participant
     */
    eid_t from() const;

private:
    eid_t from_;
    std::vector<message> messages_;
};

} // namespace clanguml::sequence_diagram::model
