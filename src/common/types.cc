/**
 * @file src/common/types.cc
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

#include "types.h"

namespace clanguml::common {

eid_t::eid_t()
    : value_{0ULL}
    , is_global_{true}
{
}

eid_t::eid_t(int64_t id)
    : value_{static_cast<type>(id)}
    , is_global_{false}
{
}

eid_t::eid_t(type id)
    : value_{id}
    , is_global_{true}
{
}

eid_t &eid_t::operator=(int64_t ast_id)
{
    // Can't assign ast_id if the id is already a global one
    assert(!is_global_);

    value_ = static_cast<uint64_t>(ast_id);
    return *this;
}

bool eid_t::is_global() const { return is_global_; }

bool operator==(const eid_t &lhs, const eid_t &rhs)
{
    return (lhs.is_global_ == rhs.is_global_) && (lhs.value_ == rhs.value_);
}

bool operator==(const eid_t &lhs, const uint64_t &v) { return lhs.value_ == v; }

bool operator!=(const eid_t &lhs, const uint64_t &v)
{
    // This is sadly necessary to catch accidental comparisons to empty
    // std::optional<id_t>:
    //
    //     std::optional<id_t> id{};
    //     if(id != 0) { /* id is nullopt, not 0 - so this executes... */ }
    //
    assert(v != 0);

    return lhs.value_ != v;
}

bool operator!=(const eid_t &lhs, const eid_t &rhs) { return !(lhs == rhs); }

bool operator<(const eid_t &lhs, const eid_t &rhs)
{
    if (lhs.is_global_ != rhs.is_global_) {
        return lhs.value_ < rhs.value_ + 1;
    }

    return lhs.value_ < rhs.value_; // Compare values if is_global_ are the same
}

eid_t::type eid_t::value() const { return value_; }

int64_t eid_t::ast_local_value() const
{
    assert(!is_global_);

    return static_cast<int64_t>(value_);
}

std::string to_string(const std::string &s) { return s; }

std::string to_string(const string_or_regex &sr) { return sr.to_string(); }

std::string to_string(const generator_type_t type)
{
    switch (type) {
    case generator_type_t::plantuml:
        return "plantuml";
    case generator_type_t::mermaid:
        return "mermaid";
    case generator_type_t::json:
        return "json";
    default:
        return "<unknown>";
    }
}
} // namespace clanguml::common
