/**
 * @file src/class_diagram/model/class_member_base.cc
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

#include "class_member_base.h"

namespace clanguml::class_diagram::model {

class_member_base::class_member_base(common::model::access_t access,
    const std::string &name, const std::string &type)
    : class_element{access, name, type}
{
}

bool class_member_base::is_static() const { return is_static_; }

void class_member_base::is_static(bool is_static) { is_static_ = is_static; }

void class_member_base::set_destination_multiplicity(std::optional<size_t> m)
{
    destination_multiplicity_ = m;
}

std::optional<size_t> class_member_base::destination_multiplicity() const
{
    return destination_multiplicity_;
}

} // namespace clanguml::class_diagram::model
