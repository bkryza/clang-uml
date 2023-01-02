/**
 * src/sequence_diagram/model/activity.h
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

#include "message.h"
#include "participant.h"

#include <string>
#include <vector>

namespace clanguml::sequence_diagram::model {

class activity {
public:
    activity(common::model::diagram_element::id_t id);

    void add_message(message m);

    std::vector<message> &messages();

    const std::vector<message> &messages() const;

    void set_from(common::model::diagram_element::id_t f);

    common::model::diagram_element::id_t from() const;

private:
    common::model::diagram_element::id_t from_;
    std::vector<message> messages_;
};

} // namespace clanguml::sequence_diagram::model
