/**
 * src/class_diagram/model/element.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

#include "element.h"

#include "util/util.h"

namespace clanguml::class_diagram::model {

std::atomic_uint64_t element::m_nextId = 1;

element::element(const std::vector<std::string> &using_namespaces)
    : using_namespaces_{using_namespaces}
    , m_id{m_nextId++}
{
}

std::string element::alias() const { return fmt::format("C_{:010}", m_id); }

void element::add_relationship(class_relationship &&cr)
{
    if (cr.destination().empty()) {
        LOG_WARN("Skipping relationship '{}' - {} - '{}' due empty "
                 "destination",
            cr.destination(), to_string(cr.type()), full_name(true));
        return;
    }

    auto it = std::find(relationships_.begin(), relationships_.end(), cr);
    if (it == relationships_.end())
        relationships_.emplace_back(std::move(cr));
}

void element::set_using_namespaces(const std::vector<std::string> &un)
{
    using_namespaces_ = un;
}

const std::vector<std::string> &element::using_namespaces() const
{
    return using_namespaces_;
}

std::vector<class_relationship> &element::relationships()
{
    return relationships_;
}

const std::vector<class_relationship> &element::relationships() const
{
    return relationships_;
}
}
