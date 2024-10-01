/**
 * @file src/class_diagram/model/objc_interface.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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

#include "objc_interface.h"

#include "common/model/filters/diagram_filter.h"
#include "util/util.h"

#include <sstream>

namespace clanguml::class_diagram::model {

objc_interface::objc_interface(const common::model::namespace_ &using_namespace)
    : element{using_namespace}
{
}

bool operator==(const objc_interface &l, const objc_interface &r)
{
    return l.id() == r.id();
}

std::string objc_interface::full_name(bool /*relative*/) const
{
    return name();
}

bool objc_interface::is_protocol() const { return is_protocol_; }

void objc_interface::is_protocol(bool ip) { is_protocol_ = ip; }

bool objc_interface::is_category() const { return is_category_; }

void objc_interface::is_category(bool cat) { is_category_ = cat; }

void objc_interface::add_member(objc_member &&member)
{
    members_.emplace_back(std::move(member));
}

void objc_interface::add_method(objc_method &&method)
{
    methods_.emplace_back(std::move(method));
}

const std::vector<objc_member> &objc_interface::members() const
{
    return members_;
}

const std::vector<objc_method> &objc_interface::methods() const
{
    return methods_;
}

std::optional<std::string> objc_interface::doxygen_link() const
{
    return std::nullopt;
}
} // namespace clanguml::class_diagram::model