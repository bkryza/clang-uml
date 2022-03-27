/**
 * src/common/model/diagram_filter.h
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

#include "diagram_filter.h"

namespace clanguml::common::model {

bool filter_visitor::match(const diagram &d, const common::model::element &e)
{
    return false;
}

bool filter_visitor::match(
    const diagram &d, const common::model::relationship_t &r)
{
    return false;
}

bool filter_visitor::match(const diagram &d, const common::model::scope_t &r)
{
    return false;
}

bool filter_visitor::match(
    const diagram &d, const common::model::namespace_ &ns)
{
    return false;
}

template <>
bool diagram_filter::should_include<std::string>(const std::string &name)
{
    auto [ns, n] = cx::util::split_ns(name);

    return should_include(ns, n);
}

}
