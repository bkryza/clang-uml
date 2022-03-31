/**
 * src/class_diagram/model/class_method.h
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
#include "method_parameter.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

class class_method : public class_element {
public:
    class_method(common::model::access_t access, const std::string &name,
        const std::string &type);

    bool is_pure_virtual() const;
    void is_pure_virtual(bool is_pure_virtual);

    bool is_virtual() const;
    void is_virtual(bool is_virtual);

    bool is_const() const;
    void is_const(bool is_const);

    bool is_defaulted() const;
    void is_defaulted(bool is_defaulted);

    bool is_static() const;
    void is_static(bool is_static);

    const std::vector<method_parameter> &parameters() const;
    void add_parameter(method_parameter &&parameter);

private:
    std::vector<method_parameter> parameters_;
    bool is_pure_virtual_{false};
    bool is_virtual_{false};
    bool is_const_{false};
    bool is_defaulted_{false};
    bool is_static_{false};
};
}
