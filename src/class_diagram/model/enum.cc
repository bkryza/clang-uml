/**
 * src/class_diagram/model/enum.cc
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

#include "enum.h"

#include "util/util.h"

#include <sstream>

namespace clanguml::class_diagram::model {

enum_::enum_(const std::vector<std::string> &using_namespaces)
    : element{using_namespaces}
{
}

bool operator==(const enum_ &l, const enum_ &r)
{
    return (l.get_namespace() == r.get_namespace()) && (l.name() == r.name());
}

std::string enum_::full_name(bool relative) const
{
    using namespace clanguml::util;

    std::ostringstream ostr;
    if (relative)
        ostr << ns_relative(using_namespaces(), name());
    else
        ostr << name();

    return ostr.str();
}

std::vector<std::string> &enum_::constants() { return constants_; }

const std::vector<std::string> &enum_::constants() const { return constants_; }

}
