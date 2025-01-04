/**
 * @file src/class_diagram/model/objc_method.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include "objc_method.h"

namespace clanguml::class_diagram::model {

objc_method::objc_method(common::model::access_t access,
    const std::string &name, const std::string &type)
    : class_method_base{access, name, type}
{
    set_display_name(name);
}

void objc_method::is_optional(bool io) { is_optional_ = io; }

bool objc_method::is_optional() const { return is_optional_; }

} // namespace clanguml::class_diagram::model