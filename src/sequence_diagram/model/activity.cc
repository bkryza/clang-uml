/**
 * @file src/sequence_diagram/model/activity.cc
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

#include "activity.h"

namespace clanguml::sequence_diagram::model {

activity::activity(eid_t id)
    : from_{id}
{
}

void activity::add_message(message m) { messages_.emplace_back(std::move(m)); }

std::vector<message> &activity::messages() { return messages_; }

const std::vector<message> &activity::messages() const { return messages_; }

eid_t activity::from() const { return from_; }

} // namespace clanguml::sequence_diagram::model
