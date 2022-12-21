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

#include <spdlog/spdlog.h>

#include <class_diagram/model/template_parameter.h>
#include <list>

namespace clanguml::cx::util {

std::pair<common::model::namespace_, std::string> split_ns(
    const std::string &full_name)
{
    assert(!full_name.empty());

    auto name_before_template = ::clanguml::util::split(full_name, "<")[0];
    auto ns = common::model::namespace_{
        ::clanguml::util::split(name_before_template, "::")};
    auto name = ns.name();
    ns.pop_back();
    return {ns, name};
}

std::vector<class_diagram::model::template_parameter>
parse_unexposed_template_params(const std::string &params,
    const std::function<std::string(const std::string &)> &ns_resolve,
    int depth)
{
    using class_diagram::model::template_parameter;

    std::vector<template_parameter> res;

    auto it = params.begin();
    while (std::isspace(*it) != 0)
        ++it;

    std::string type{};
    std::vector<template_parameter> nested_params;
    bool complete_class_template_argument{false};

    while (it != params.end()) {
        if (*it == '<') {
            int nested_level{0};
            auto bracket_match_begin = it + 1;
            auto bracket_match_end = bracket_match_begin;
            while (bracket_match_end != params.end()) {
                if (*bracket_match_end == '<') {
                    nested_level++;
                }
                else if (*bracket_match_end == '>') {
                    if (nested_level > 0)
                        nested_level--;
                    else
                        break;
                }
                else {
                }
                bracket_match_end++;
            }

            std::string nested_params_str(
                bracket_match_begin, bracket_match_end);

            nested_params = parse_unexposed_template_params(
                nested_params_str, ns_resolve, depth + 1);

            if (nested_params.empty())
                nested_params.emplace_back(
                    template_parameter{nested_params_str});

            it = bracket_match_end - 1;
        }
        else if (*it == '>') {
            complete_class_template_argument = true;
            if (depth == 0) {
                break;
            }
        }
        else if (*it == ',') {
            complete_class_template_argument = true;
        }
        else {
            type += *it;
        }
        if (complete_class_template_argument) {
            template_parameter t;
            t.set_type(ns_resolve(clanguml::util::trim(type)));
            type = "";
            for (auto &&param : nested_params)
                t.add_template_param(std::move(param));

            res.emplace_back(std::move(t));
            complete_class_template_argument = false;
        }
        it++;
    }

    if (!type.empty()) {
        template_parameter t;
        t.set_type(ns_resolve(clanguml::util::trim(type)));
        type = "";
        for (auto &&param : nested_params)
            t.add_template_param(std::move(param));

        res.emplace_back(std::move(t));
    }

    return res;
}

} // namespace clanguml::cx::util
