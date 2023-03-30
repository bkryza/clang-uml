/**
 * src/class_diagram/model/class_element.h
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

#include "common/model/decorated_element.h"
#include "common/model/source_location.h"

#include <inja/inja.hpp>

#include <string>

namespace clanguml::class_diagram::model {

class class_element : public common::model::decorated_element,
                      public common::model::source_location {
public:
    class_element(
        common::model::access_t scope, std::string name, std::string type);

    virtual ~class_element() = default;

    common::model::access_t access() const;
    std::string name() const;
    void set_name(const std::string &name);

    std::string type() const;
    void set_type(const std::string &type);

    virtual inja::json context() const;

private:
    common::model::access_t access_;
    std::string name_;
    std::string type_;
};

} // namespace clanguml::class_diagram::model
