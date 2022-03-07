/**
 * src/cx/util.cc
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

#include "cx/util.h"
#include "util/util.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_namespace.hpp>
#include <cppast/cpp_template.hpp>
#include <spdlog/spdlog.h>

#include <list>

namespace clanguml {
namespace cx {
namespace util {

std::string to_string(CXString &&cxs)
{
    std::string r{clang_getCString(cxs)};
    clang_disposeString(cxs);
    return r;
}

std::string to_string(cppast::cpp_type_kind t)
{
    using namespace cppast;
    switch (t) {
    case cpp_type_kind::builtin_t:
        return "builtin";
    case cpp_type_kind::user_defined_t:
        return "user_defined";
    case cpp_type_kind::auto_t:
        return "auto";
    case cpp_type_kind::decltype_t:
        return "decltype";
    case cpp_type_kind::decltype_auto_t:
        return "decltype_auto";
    case cpp_type_kind::cv_qualified_t:
        return "cv_qualified";
    case cpp_type_kind::pointer_t:
        return "pointer";
    case cpp_type_kind::reference_t:
        return "reference";
    case cpp_type_kind::array_t:
        return "array";
    case cpp_type_kind::function_t:
        return "function";
    case cpp_type_kind::member_function_t:
        return "member_function";
    case cpp_type_kind::member_object_t:
        return "member_object";
    case cpp_type_kind::template_parameter_t:
        return "template_parameter";
    case cpp_type_kind::template_instantiation_t:
        return "template_instantiation";
    case cpp_type_kind::dependent_t:
        return "dependent";
    case cpp_type_kind::unexposed_t:
        return "unexposed";
    default:
        return "invalid";
    }
}

std::string full_name(
    const common::model::namespace_ &current_ns, const cppast::cpp_entity &e)
{
    if (e.name().empty())
        return "";
    else if (cppast::is_parameter(e.kind()))
        // parameters don't have a full name
        return e.name();

    std::vector<std::string> fn;

    for (const auto &ns : current_ns) {
        if (!ns.empty())
            fn.push_back(ns);
    }

    fn.push_back(e.name());

    return fmt::format("{}", fmt::join(fn, "::"));
}

std::string full_name(const cppast::cpp_type &t,
    const cppast::cpp_entity_index &idx, bool inside_class)
{
    std::string t_ns;
    if (!inside_class) {
        t_ns = ns(cppast::remove_cv(unreferenced(t)), idx);
    }

    auto t_name = cppast::to_string(t);

    if (t_ns.size() > 0 &&
        t_name.substr(0, t_name.find("<")).find("::") == std::string::npos)
        return t_ns + "::" + t_name;

    return cppast::to_string(t);
}

std::string ns(const cppast::cpp_entity &e)
{
    std::vector<std::string> res{};

    auto it = e.parent();
    while (it) {
        if (it.value().kind() == cppast::cpp_entity_kind::namespace_t) {
            if (!it.value().name().empty())
                res.push_back(it.value().name());
        }
        it = it.value().parent();
    }

    if (res.empty())
        return "";

    std::reverse(res.begin(), res.end());

    return fmt::format("{}", fmt::join(res, "::"));
}

type_safe::optional_ref<const cppast::cpp_namespace> entity_ns(
    const cppast::cpp_entity &e)
{
    std::vector<std::string> res{};

    if (e.kind() == cppast::cpp_entity_kind::namespace_t)
        return type_safe::optional_ref<const cppast::cpp_namespace>(
            static_cast<const cppast::cpp_namespace &>(e));

    auto it = e.parent();
    while (it) {
        if (it.value().kind() == cppast::cpp_entity_kind::namespace_t) {
            return type_safe::optional_ref<const cppast::cpp_namespace>(
                static_cast<const cppast::cpp_namespace &>(it.value()));
        }
        it = it.value().parent();
    }

    return {};
}

bool is_inside_class(const cppast::cpp_entity &e)
{
    auto it = e.parent();
    while (it) {
        if (it.value().kind() == cppast::cpp_entity_kind::class_t) {
            return true;
        }
        it = it.value().parent();
    }
    return false;
}

std::pair<common::model::namespace_, std::string> split_ns(
    const std::string &full_name)
{
    auto name_before_template = ::clanguml::util::split(full_name, "<")[0];
    auto ns = common::model::namespace_{
        ::clanguml::util::split(name_before_template, "::")};
    auto name = ns.name();
    ns.pop_back();
    return {ns, name};
}

std::string ns(const cppast::cpp_type &t, const cppast::cpp_entity_index &idx)
{
    if (t.kind() == cppast::cpp_type_kind::user_defined_t &&
        (static_cast<const cppast::cpp_user_defined_type &>(t)
                .entity()
                .get(idx)
                .size() > 0)) {
        // If this is a user defined type - return the namespace of the
        // entity
        return ns(static_cast<const cppast::cpp_user_defined_type &>(t)
                      .entity()
                      .get(idx)[0]
                      .get());
    }
    else if (t.kind() == cppast::cpp_type_kind::template_instantiation_t) {
        if (static_cast<const cppast::cpp_template_instantiation_type &>(t)
                .primary_template()
                .get(idx)
                .size() > 0) {
            return ns(
                static_cast<const cppast::cpp_template_instantiation_type &>(t)
                    .primary_template()
                    .get(idx)[0]
                    .get());
        }
        else
            return "";
    }
    else {
        auto canon = cppast::to_string(t.canonical());
        auto full_name = canon.substr(0, canon.find("<"));
        if (full_name.empty()) {
            return "";
        }
        else if (canon.find("type-parameter-") == std::string::npos) {
            // This is an easy case, canonical representation contains full
            // namespace
            auto ns_toks = clanguml::util::split(full_name, "::");
            if (ns_toks.size() > 0)
                ns_toks.pop_back();
            return fmt::format(
                "{}", fmt::join(ns_toks.begin(), ns_toks.end(), "::"));
        }
        else {
            // This is a bug/feature in libclang, where canonical representation
            // of a template type with incomplete specialization doesn't have a
            // full namespace. We have to extract it from the primary template
            const auto &primary_template =
                static_cast<const cppast::cpp_template_instantiation_type &>(t)
                    .primary_template();
            if (!primary_template.is_overloaded()) {
                LOG_WARN(
                    "Cannot establish namespace for ", cppast::to_string(t));
                return "";
            }
            return ns(primary_template.get(idx)[0].get());
        }
    }
}

std::string fully_prefixed(
    const common::model::namespace_ &current_ns, const cppast::cpp_entity &e)
{
    if (e.name().find("::") != std::string::npos) {
        // the name already contains namespace, but it could be not
        // absolute, i.e. relative to some supernamespace of current
        // namespace context
        std::list<std::string> res;

        for (const auto &n : clanguml::util::split(e.name(), "::"))
            res.push_back(n);

        std::list<std::string> prefix_ns;
        for (const auto &n : current_ns) {
            if (!n.empty() && n != res.front())
                prefix_ns.push_back(n);
            else
                break;
        }

        prefix_ns.reverse();
        for (const auto &n : prefix_ns)
            res.push_front(n);

        return fmt::format("{}", fmt::join(res, "::"));
    }

    std::vector<std::string> res{e.name()};

    auto it = e.parent();
    while (it) {
        if (it.value().kind() == cppast::cpp_entity_kind::namespace_t) {
            if (!it.value().name().empty())
                res.push_back(it.value().name());
        }
        it = it.value().parent();
    }

    return fmt::format("{}", fmt::join(res.rbegin(), res.rend(), "::"));
}

const cppast::cpp_type &unreferenced(const cppast::cpp_type &t)
{
    if (t.kind() == cppast::cpp_type_kind::pointer_t)
        return unreferenced(
            static_cast<const cppast::cpp_pointer_type &>(t).pointee());
    else if (t.kind() == cppast::cpp_type_kind::reference_t)
        return unreferenced(
            static_cast<const cppast::cpp_reference_type &>(t).referee());

    return t;
}
} // namespace util
} // namespace cx
} // namespace clanguml
