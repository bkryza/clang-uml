/**
 * @file src/common/model/diagram.cc
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

#include "diagram.h"

#include "diagram_filter.h"
#include "namespace.h"

namespace clanguml::common::model {

diagram::diagram() = default;

diagram::~diagram() = default;

diagram::diagram(diagram &&) noexcept = default;

diagram &diagram::operator=(diagram &&) noexcept = default;

common::optional_ref<clanguml::common::model::diagram_element>
diagram::get_with_namespace(const std::string &name, const namespace_ &ns) const
{
    auto element_opt = get(name);

    if (!element_opt) {
        // If no element matches, try to prepend the 'using_namespace'
        // value to the element and search again
        auto fully_qualified_name = ns | name;
        element_opt = get(fully_qualified_name.to_string());
    }

    return element_opt;
}

std::string diagram::name() const { return name_; }

void diagram::set_name(const std::string &name) { name_ = name; }

void diagram::set_filter(std::unique_ptr<diagram_filter> filter)
{
    filter_ = std::move(filter);
}

void diagram::set_complete(bool complete) { complete_ = complete; }

bool diagram::complete() const { return complete_; }

void diagram::finalize() { }

bool diagram::should_include(const element &e) const
{
    if (filter_.get() == nullptr)
        return true;

    return filter_->should_include(e) &&
        filter_->should_include(dynamic_cast<const source_location &>(e));
}

bool diagram::should_include(const namespace_ &ns) const
{
    if (filter_.get() == nullptr)
        return true;

    return filter_->should_include(ns);
}

bool diagram::should_include(relationship r) const
{
    return should_include(r.type());
}

bool diagram::should_include(const relationship_t r) const
{
    if (filter_.get() == nullptr)
        return true;

    return filter_->should_include(r);
}

bool diagram::should_include(const access_t s) const
{
    if (filter_.get() == nullptr)
        return true;

    return filter_->should_include(s);
}

bool diagram::should_include(
    const namespace_ &ns, const std::string &name) const
{
    if (filter_.get() == nullptr)
        return true;

    return filter_->should_include(ns, name);
}

bool diagram::should_include(const common::model::source_file &f) const
{
    if (filter_.get() == nullptr)
        return true;

    return filter_->should_include(f);
}

} // namespace clanguml::common::model