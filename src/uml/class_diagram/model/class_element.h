/**
 * src/uml/class_diagram/model/class_element.h
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

#include "decorated_element.h"
#include "enums.h"

#include <string>

namespace clanguml::class_diagram::model {

class class_element : public decorated_element {
public:
    class_element(
        scope_t scope, const std::string &name, const std::string &type);

    scope_t scope() const;
    std::string name() const;
    std::string type() const;

private:
    scope_t scope_;
    std::string name_;
    std::string type_;
};

}
