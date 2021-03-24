/**
 * src/cx/util.cc
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

#include "cx/util.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <spdlog/spdlog.h>

namespace clanguml {
namespace cx {
namespace util {

std::string to_string(CXString &&cxs)
{
    std::string r{clang_getCString(cxs)};
    clang_disposeString(cxs);
    return r;
}

std::string full_name(const cppast::cpp_entity &e)
{
    if (e.name().empty())
        return "";
    else if (cppast::is_parameter(e.kind()))
        // parameters don't have a full name
        return e.name();

    std::string scopes;

    for (auto cur = e.parent(); cur; cur = cur.value().parent())
        // prepend each scope, if there is any
        if (cur.value().kind() == cppast::cpp_entity_kind::namespace_t)
            type_safe::with(cur.value().scope_name(),
                [&](const cppast::cpp_scope_name &cur_scope) {
                    scopes = cur_scope.name() + "::" + scopes;
                });

    if (e.kind() == cppast::cpp_entity_kind::class_t) {
        auto &c = static_cast<const cppast::cpp_class &>(e);
        return scopes /*+ c.semantic_scope()*/ + c.name();
    }
    else if (e.kind() == cppast::cpp_entity_kind::class_template_t) {
        return scopes;
    }
    else
        return scopes + e.name();
}

std::string ns(const cppast::cpp_entity &e)
{
    std::vector<std::string> res{};

    auto it = e.parent();
    while (it) {
        if (it.value().kind() == cppast::cpp_entity_kind::namespace_t) {
            res.push_back(it.value().name());
        }
        it = it.value().parent();
    }

    return fmt::format("{}", fmt::join(res.rbegin(), res.rend(), "::"));
}

std::string fully_prefixed(const cppast::cpp_entity &e)
{
    std::vector<std::string> res{e.name()};

    auto it = e.parent();
    while (it) {
        if (it.value().kind() == cppast::cpp_entity_kind::namespace_t) {
            res.push_back(it.value().name());
        }
        it = it.value().parent();
    }

    return fmt::format("{}", fmt::join(res.rbegin(), res.rend(), "::"));
}
} // namespace util
} // namespace cx
} // namespace clanguml
