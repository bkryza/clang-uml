/**
 * src/uml/sequence_diagram_model.h
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace clanguml {
namespace model {
namespace sequence_diagram {

enum class message_t { kCall, kReturn };

struct message {
    message_t type;
    std::string from;
    std::string from_usr;
    std::string to;
    std::string to_usr;
    std::string message;
    std::string return_type;
    unsigned int line;
};

struct activity {
    std::string usr;
    std::string from;
    std::vector<message> messages;
};

struct diagram {
    bool started{false};
    std::string name;

    //std::map<std::string, activity> sequences;
    std::map<std::string, activity> sequences;

    void sort()
    {
        /*
        std::sort(sequence.begin(), sequence.end(),
            [](const auto &a, const auto &b) -> bool {
                if (a.from_usr == b.from_usr)
                    return a.line > b.line;

                return a.from_usr > b.from_usr;
            });
            */
    }
};
}
}
}
