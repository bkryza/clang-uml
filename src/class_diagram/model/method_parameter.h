/**
 * src/class_diagram/model/method_parameter.h
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

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

class method_parameter : public decorated_element {
public:
    void set_type(const std::string &type);
    std::string type() const;

    void set_name(const std::string &name);
    std::string name() const;

    void set_default_value(const std::string &value);
    std::string default_value() const;

    std::string to_string(
        const std::vector<std::string> &using_namespaces) const;

private:
    std::string type_;
    std::string name_;
    std::string default_value_;
};

}
