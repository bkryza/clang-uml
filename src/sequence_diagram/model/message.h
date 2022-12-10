/**
 * src/sequence_diagram/model/message.h
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

#include "common/model/enums.h"
#include "participant.h"

#include <string>
#include <vector>

namespace clanguml::sequence_diagram::model {

struct message : public common::model::diagram_element {
    message()
        : from{}
        , to{}
        , message_name{}
        , return_type{}
    {
    }

    common::model::message_t type;
    common::model::diagram_element::id_t from;
    common::model::diagram_element::id_t to;

    std::string message_name;

    std::string return_type;
};

}
