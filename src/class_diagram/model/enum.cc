/**
 * @file src/class_diagram/model/enum.cc
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

#include "enum.h"

#include "util/util.h"

#include <sstream>

namespace clanguml::class_diagram::model {

enum_::enum_(const common::model::namespace_ &using_namespace)
    : element{using_namespace}
{
}

bool operator==(const enum_ &l, const enum_ &r)
{
    return (l.get_namespace() == r.get_namespace()) && (l.name() == r.name());
}

std::string enum_::full_name(bool relative) const
{
    using namespace clanguml::util;
    using clanguml::common::model::namespace_;

    std::ostringstream ostr;
    if (relative)
        ostr << namespace_{name_and_ns()}
                    .relative_to(using_namespace())
                    .to_string();
    else
        ostr << name_and_ns();

    return ostr.str();
}

std::vector<std::string> &enum_::constants() { return constants_; }

const std::vector<std::string> &enum_::constants() const { return constants_; }

std::optional<std::string> enum_::doxygen_link() const
{
    auto name = name_and_ns();
    util::replace_all(name, "_", "__");
    util::replace_all(name, "::", "_1_1");
    return fmt::format("enum{}.html", name);
}
} // namespace clanguml::class_diagram::model
