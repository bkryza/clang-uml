/**
 * @file src/common/model/template_element.cc
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

#include "template_element.h"

namespace clanguml::common::model {

bool template_element::is_template() const { return is_template_; }

void template_element::is_template(bool is_template)
{
    is_template_ = is_template;
}

int template_element::calculate_template_specialization_match(
    const template_element &other) const
{
    int res{0};

    if (name_and_ns() != other.name_and_ns()) {
        return res;
    }

    return template_trait::calculate_template_specialization_match(other);
}

void template_element::template_specialization_found(bool found)
{
    template_specialization_found_ = found;
}

bool template_element::template_specialization_found() const
{
    return template_specialization_found_;
}

} // namespace clanguml::common::model