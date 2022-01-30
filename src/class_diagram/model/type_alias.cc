/**
 * src/class_diagram/model/type_alias.cc
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

#include "type_alias.h"

namespace clanguml::class_diagram::model {

void type_alias::set_alias(const std::string &alias) { alias_ = alias; }

std::string type_alias::alias() const { return alias_; }

void type_alias::set_underlying_type(const std::string &type)
{
    underlying_type_ = type;
}

std::string type_alias::underlying_type() const { return underlying_type_; }

}
