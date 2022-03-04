/**
 * src/sequence_diagram/visitor/translation_unit_visitor.cc
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

#include "translation_unit_visitor.h"

#include "common/model/namespace.h"
#include "translation_unit_context.h"

#include <cppast/cpp_function.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/visitor.hpp>

namespace clanguml::sequence_diagram::visitor {

translation_unit_visitor::translation_unit_visitor(
    cppast::cpp_entity_index &idx,
    clanguml::sequence_diagram::model::diagram &diagram,
    const clanguml::config::sequence_diagram &config)
    : ctx{idx, diagram, config}
{
}

void translation_unit_visitor::process_activities(const cppast::cpp_function &e)
{
    using clanguml::common::model::message_t;
    using clanguml::sequence_diagram::model::activity;
    using clanguml::sequence_diagram::model::diagram;
    using clanguml::sequence_diagram::model::message;
    using cppast::cpp_entity;
    using cppast::cpp_entity_kind;
    using cppast::cpp_function;
    using cppast::cpp_member_function;
    using cppast::cpp_member_function_call;
    using cppast::visitor_info;

    for (const auto &function_call_ptr : e.function_calls()) {
        const auto &function_call =
            static_cast<const cpp_member_function_call &>(*function_call_ptr);

        message m;
        m.type = message_t::kCall;

        if (!ctx.entity_index()
                 .lookup_definition(function_call.get_caller_id())
                 .has_value())
            continue;

        if (!ctx.entity_index()
                 .lookup_definition(function_call.get_caller_method_id())
                 .has_value())
            continue;

        if (!ctx.entity_index()
                 .lookup_definition(function_call.get_callee_id())
                 .has_value())
            continue;

        if (!ctx.entity_index()
                 .lookup_definition(function_call.get_callee_method_id())
                 .has_value())
            continue;

        const auto &caller =
            ctx.entity_index()
                .lookup_definition(function_call.get_caller_id())
                .value();
        m.from = cx::util::ns(caller) + "::" + caller.name();

        if (!ctx.config().should_include(
                common::model::namespace_{cx::util::ns(caller)}, caller.name()))
            continue;

        if (caller.kind() == cpp_entity_kind::function_t)
            m.from += "()";

        m.from_usr = type_safe::get(function_call.get_caller_method_id());

        const auto &callee =
            ctx.entity_index()
                .lookup_definition(function_call.get_callee_id())
                .value();
        m.to = cx::util::ns(callee) + "::" + callee.name();
        if (callee.kind() == cpp_entity_kind::function_t)
            m.to += "()";

        if (!ctx.config().should_include(
                common::model::namespace_{cx::util::ns(callee)}, callee.name()))
            continue;

        m.to_usr = type_safe::get(function_call.get_callee_method_id());

        const auto &callee_method =
            static_cast<const cppast::cpp_member_function &>(
                ctx.entity_index()
                    .lookup_definition(function_call.get_callee_method_id())
                    .value());

        m.message = callee_method.name();

        m.return_type = cppast::to_string(callee_method.return_type());

        if (ctx.diagram().sequences.find(m.from_usr) ==
            ctx.diagram().sequences.end()) {
            activity a;
            a.usr = m.from_usr;
            a.from = m.from;
            ctx.diagram().sequences.insert({m.from_usr, std::move(a)});
        }

        LOG_DBG("Adding sequence {} -{}()-> {}", m.from, m.message, m.to);

        ctx.diagram().sequences[m.from_usr].messages.emplace_back(std::move(m));
    }
}

void translation_unit_visitor::operator()(const cppast::cpp_entity &file)
{
    using cppast::cpp_entity;
    using cppast::cpp_entity_kind;
    using cppast::cpp_function;
    using cppast::cpp_member_function;
    using cppast::cpp_member_function_call;
    using cppast::visitor_info;

    cppast::visit(file, [&, this](const cpp_entity &e, visitor_info info) {
        if (e.kind() == cpp_entity_kind::function_t) {
            const auto &function = static_cast<const cpp_function &>(e);
            process_activities(function);
        }
        else if (e.kind() == cpp_entity_kind::member_function_t) {
            const auto &member_function = static_cast<const cpp_function &>(e);
            process_activities(member_function);
        }
    });
}
}
