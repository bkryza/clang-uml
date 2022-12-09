/**
 * src/sequence_diagram/model/participant.cc
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

#include "participant.h"

namespace clanguml::sequence_diagram::model {

std::ostringstream &template_trait::render_template_params(
    std::ostringstream &ostr, const common::model::namespace_ &using_namespace,
    bool relative) const
{
    using clanguml::common::model::namespace_;

    if (!templates_.empty()) {
        std::vector<std::string> tnames;
        std::vector<std::string> tnames_simplified;

        std::transform(templates_.cbegin(), templates_.cend(),
            std::back_inserter(tnames),
            [ns = using_namespace, relative](
                const auto &tmplt) { return tmplt.to_string(ns, relative); });

        ostr << fmt::format("<{}>", fmt::join(tnames, ","));
    }

    return ostr;
}

void template_trait::set_base_template(const std::string &full_name)
{
    base_template_full_name_ = full_name;
}

std::string template_trait::base_template() const
{
    return base_template_full_name_;
}

void template_trait::add_template(
    class_diagram::model::template_parameter tmplt)
{
    templates_.push_back(std::move(tmplt));
}

bool template_trait::is_implicit() const { return is_implicit_; }

void template_trait::set_implicit(bool implicit) { is_implicit_ = implicit; }

const std::vector<class_diagram::model::template_parameter> &
template_trait::templates() const
{
    return templates_;
}

int template_trait::calculate_template_specialization_match(
    const template_trait &other, const std::string &full_name) const
{
    int res{};

    //    std::string left = name_and_ns();
    //    // TODO: handle variadic templates
    //    if ((name_and_ns() != full_name) ||
    //        (templates().size() != other.templates().size())) {
    //        return res;
    //    }

    // Iterate over all template arguments
    for (auto i = 0U; i < other.templates().size(); i++) {
        const auto &template_arg = templates().at(i);
        const auto &other_template_arg = other.templates().at(i);

        if (template_arg == other_template_arg) {
            res++;
        }
        else if (other_template_arg.is_specialization_of(template_arg)) {
            continue;
        }
        else {
            res = 0;
            break;
        }
    }

    return res;
}

std::string participant::to_string() const
{
    return fmt::format(
        "Participant '{}': id={} name={}", type_name(), id(), full_name(false));
}

class_::class_(const common::model::namespace_ &using_namespace)
    : participant{using_namespace}
{
}

bool class_::is_struct() const { return is_struct_; }

void class_::is_struct(bool is_struct) { is_struct_ = is_struct; }

bool class_::is_template() const { return is_template_; }

void class_::is_template(bool is_template) { is_template_ = is_template; }

bool class_::is_template_instantiation() const
{
    return is_template_instantiation_;
}

void class_::is_template_instantiation(bool is_template_instantiation)
{
    is_template_instantiation_ = is_template_instantiation;
}

std::string class_::full_name_no_ns() const
{
    using namespace clanguml::util;

    std::ostringstream ostr;

    ostr << name();

    render_template_params(ostr, using_namespace(), false);

    return ostr.str();
}

std::string class_::full_name(bool relative) const
{
    using namespace clanguml::util;
    using clanguml::common::model::namespace_;

    std::ostringstream ostr;

    ostr << name_and_ns();
    render_template_params(ostr, using_namespace(), relative);

    std::string res;

    if (relative)
        res = using_namespace().relative(ostr.str());
    else
        res = ostr.str();

    if (res.empty())
        return "<<anonymous>>";

    return res;
}

bool class_::is_alias() const { return is_alias_; }

void class_::is_alias(bool alias) { is_alias_ = alias; }

bool class_::is_lambda() const { return is_lambda_; }

void class_::is_lambda(bool is_lambda) { is_lambda_ = is_lambda; }

bool operator==(const class_ &l, const class_ &r) { return l.id() == r.id(); }

function::function(const common::model::namespace_ &using_namespace)
    : participant{using_namespace}
{
}

std::string function::full_name(bool relative) const
{
    return fmt::format("{}({}){}", element::full_name(relative),
        fmt::join(parameters_, ","), is_const() ? " const" : "");
}

std::string function::full_name_no_ns() const
{
    return fmt::format("{}({}){}", element::full_name_no_ns(),
        fmt::join(parameters_, ","), is_const() ? " const" : "");
}

std::string function::message_name(message_render_mode mode) const
{
    if (mode == message_render_mode::no_arguments) {
        return fmt::format("{}(){}", name(), is_const() ? " const" : "");
    }

    return fmt::format("{}({}){}", name(), fmt::join(parameters_, ","),
        is_const() ? " const" : "");
}

bool function::is_const() const { return is_const_; }

void function::is_const(bool c) { is_const_ = c; }

bool function::is_void() const { return is_void_; }

void function::is_void(bool v) { is_void_ = v; }

void function::add_parameter(const std::string &a) { parameters_.push_back(a); }

const std::vector<std::string> &function::parameters() const
{
    return parameters_;
}

method::method(const common::model::namespace_ &using_namespace)
    : function{using_namespace}
{
}

const std::string method::method_name() const { return method_name_; }

std::string method::alias() const
{
    assert(class_id_ >= 0);

    return fmt::format("C_{:022}", class_id_);
}

void method::set_method_name(const std::string &name) { method_name_ = name; }

void method::set_class_id(diagram_element::id_t id) { class_id_ = id; }

void method::set_class_full_name(const std::string &name)
{
    class_full_name_ = name;
}

const auto &method::class_full_name() const { return class_full_name_; }

std::string method::full_name(bool /*relative*/) const
{
    return fmt::format("{}::{}({}){}", class_full_name(), method_name(),
        fmt::join(parameters(), ","), is_const() ? " const" : "");
}

std::string method::message_name(message_render_mode mode) const
{
    if (mode == message_render_mode::no_arguments) {
        return fmt::format("{}(){}", method_name(), is_const() ? " const" : "");
    }

    return fmt::format("{}({}){}", method_name(), fmt::join(parameters(), ","),
        is_const() ? " const" : "");
}

class_::diagram_element::id_t method::class_id() const { return class_id_; }

std::string method::to_string() const
{
    return fmt::format("Participant '{}': id={}, name={}, class_id={}",
        type_name(), id(), full_name(false), class_id());
}

function_template::function_template(
    const common::model::namespace_ &using_namespace)
    : function{using_namespace}
{
}

std::string function_template::full_name(bool relative) const
{
    using namespace clanguml::util;
    using clanguml::common::model::namespace_;

    std::ostringstream ostr;

    ostr << name_and_ns();
    render_template_params(ostr, using_namespace(), relative);

    ostr << fmt::format(
        "({}){}", fmt::join(parameters(), ","), is_const() ? " const" : "");

    std::string res;

    if (relative)
        res = using_namespace().relative(ostr.str());
    else
        res = ostr.str();

    if (res.empty())
        return "<<anonymous>>";

    return res;
}

std::string function_template::full_name_no_ns() const
{
    using namespace clanguml::util;

    std::ostringstream ostr;

    ostr << name();

    render_template_params(ostr, using_namespace(), false);

    ostr << fmt::format(
        "({}){}", fmt::join(parameters(), ","), is_const() ? " const" : "");

    return ostr.str();
}

std::string function_template::message_name(message_render_mode mode) const
{
    std::ostringstream s;
    render_template_params(s, using_namespace(), true);
    std::string template_params = s.str();

    if (mode == message_render_mode::no_arguments) {
        return fmt::format(
            "{}{}(){}", name(), template_params, is_const() ? " const" : "");
    }

    return fmt::format("{}{}({}){}", name(), template_params,
        fmt::join(parameters(), ","), is_const() ? " const" : "");
}

}