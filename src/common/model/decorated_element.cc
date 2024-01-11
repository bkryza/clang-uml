/**
 * @file src/common/model/decorated_element.cc
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

#include "decorated_element.h"

namespace clanguml::common::model {

bool decorated_element::skip() const
{
    return std::any_of(
        decorators_.begin(), decorators_.end(), [](const auto &d) {
            return std::dynamic_pointer_cast<decorators::skip>(d) != nullptr;
        });
}

bool decorated_element::skip_relationship() const
{
    return std::any_of(
        decorators_.begin(), decorators_.end(), [](const auto &d) {
            return std::dynamic_pointer_cast<decorators::skip_relationship>(
                       d) != nullptr;
        });
}

std::pair<relationship_t, std::string>
decorated_element::get_relationship() const
{
    for (const auto &d : decorators_)
        if (std::dynamic_pointer_cast<decorators::association>(d))
            return {relationship_t::kAssociation,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};
        else if (std::dynamic_pointer_cast<decorators::aggregation>(d))
            return {relationship_t::kAggregation,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};
        else if (std::dynamic_pointer_cast<decorators::composition>(d))
            return {relationship_t::kComposition,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};

    return {relationship_t::kNone, ""};
}

std::string decorated_element::style_spec() const
{
    for (const auto &d : decorators_)
        if (std::dynamic_pointer_cast<decorators::style>(d))
            return std::dynamic_pointer_cast<decorators::style>(d)->spec;

    return "";
}

const std::vector<std::shared_ptr<decorators::decorator>> &
decorated_element::decorators() const
{
    return decorators_;
}

void decorated_element::add_decorators(
    const std::vector<std::shared_ptr<decorators::decorator>> &decorators)
{
    for (const auto &d : decorators) {
        decorators_.push_back(d);
    }
}

void decorated_element::append(const decorated_element &de)
{
    for (const auto &d : de.decorators()) {
        decorators_.push_back(d);
    }
}

std::optional<comment_t> decorated_element::comment() const { return comment_; }

void decorated_element::set_comment(const comment_t &c) { comment_ = c; }

std::optional<std::string> decorated_element::doxygen_link() const
{
    return std::nullopt;
}

} // namespace clanguml::common::model
