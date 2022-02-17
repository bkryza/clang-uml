/**
 * src/common/model/class.h
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

#include "package.h"

#include "util/util.h"

#include <sstream>

namespace clanguml::common::model {
package::package(const std::vector<std::string> &using_namespaces)
    : element{using_namespaces}
{
}

std::string package::full_name(bool relative) const
{
    auto fn = get_namespace();
    auto ns = using_namespaces();

    if (relative && (fn.size() >= ns.size())) {
        if (util::starts_with(fn, ns))
            fn = std::vector<std::string>(fn.begin() + ns.size(), fn.end());
    }

    fn.push_back(name());

    return fmt::format("{}", fmt::join(fn, "::"));
}

bool operator==(const package &l, const package &r)
{
    return l.full_name(false) == r.full_name(false);
}

bool package::is_deprecated() const { return is_deprecated_; }

void package::set_deprecated(bool deprecated) { is_deprecated_ = deprecated; }
}