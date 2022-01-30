/**
 * src/class_diagram/model/class_member.h
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

#include "class_element.h"

#include <string>

namespace clanguml::class_diagram::model {

class class_member : public class_element {
public:
    class_member(common::model::scope_t scope, const std::string &name,
        const std::string &type);

    bool is_relationship() const;
    void is_relationship(bool is_relationship);

    bool is_static() const;
    void is_static(bool is_static);

private:
    bool is_relationship_{false};
    bool is_static_{false};
};

}
