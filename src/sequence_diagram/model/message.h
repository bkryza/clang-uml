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

#include <string>
#include <vector>

namespace clanguml::sequence_diagram::model {

struct message {
    message()
        : from{}
        , from_name{}
        , to{}
        , to_name{}
        , message_name{}
        , return_type{}
        , line{}
    {
    }

    common::model::message_t type;

    /// Source participant id
    std::uint_least64_t from;
    std::string from_name;
    //    std::uint_least64_t from_usr{};

    /// Target participant id
    std::uint_least64_t to;
    std::string to_name;

    //    std::int64_t to_usr{};

    std::string message_name;

    std::string return_type;

    unsigned int line;
};

}
