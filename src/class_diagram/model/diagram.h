/**
 * src/class_diagram/model/diagram.h
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

#include "class.h"
#include "common/model/diagram.h"
#include "enum.h"
#include "type_alias.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

class diagram : public clanguml::common::model::diagram {
public:
    const std::vector<class_> classes() const;

    const std::vector<enum_> enums() const;

    bool has_class(const class_ &c) const;

    void add_type_alias(type_alias &&ta);

    void add_class(class_ &&c);

    void add_enum(enum_ &&e);

    std::string to_alias(const std::string &full_name) const;

private:
    std::vector<class_> classes_;
    std::vector<enum_> enums_;
    std::map<std::string, type_alias> type_aliases_;
};
}
