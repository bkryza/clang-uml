/**
 * @file src/common/model/diagram.cc
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

#include "diagram.h"

#include "filters/diagram_filter.h"
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

void diagram::finalize()
{
    // Remove elements that do not match the filter
    apply_filter();
    filtered_ = true;
}

bool diagram::should_include(const element &e) const
{
    if (filtered_)
        return true;

    if (filter_.get() == nullptr)
        return true;

    // In the basic mode, apply the paths filter as soon as possible
    // to limit processing unnecessary files
    if (filter_->mode() == filter_mode_t::basic) {
        return filter_->should_include(e);
    }

    // In advanced mode, we have to wait until the diagram model is complete
    // before we can filter anything out
    if (filter_->mode() == filter_mode_t::advanced && !complete()) {
        return true;
    }

    return filter_->should_include(e);
}

bool diagram::should_include(const namespace_ &ns) const
{
    if (filtered_)
        return true;

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

void diagram::add_relationship(relationship &&cr)
{
    if ((cr.type() == relationship_t::kInstantiation) &&
        (cr.destination() == id())) {
        LOG_DBG("Skipping self instantiation relationship for {}",
            cr.destination());
        return;
    }

    if (!util::contains(relationships_, cr)) {
        LOG_DBG("Adding relationship from: '{}' ({}) - {} - '{}'", id(),
            full_name(true), to_string(cr.type()), cr.destination());

        relationships_.emplace_back(std::move(cr));
    }
}

std::vector<relationship> &diagram::relationships()
{
    return relationships_;
}

const std::vector<relationship> &diagram::relationships() const
{
    return relationships_;
}

void diagram::remove_duplicate_relationships()
{
    std::vector<relationship> unique_relationships;

    for (auto &r : relationships_) {
        if (!util::contains(unique_relationships, r)) {
            unique_relationships.emplace_back(r);
        }
    }

    std::swap(relationships_, unique_relationships);
}

void diagram::apply_filter(
    const diagram_filter &filter, const std::set<eid_t> &removed)
{
    common::model::apply_filter(relationships(), filter);

    auto &rels = relationships();
    rels.erase(std::remove_if(std::begin(rels), std::end(rels),
                   [&removed](auto &&r) {
                       return removed.count(r.destination()) > 0;
                   }),
        std::end(rels));
}

} // namespace clanguml::common::model