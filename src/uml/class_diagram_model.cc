/**
 * src/uml/class_diagram_model.cc
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

#include "class_diagram_model.h"

namespace clanguml {
namespace model {
namespace class_diagram {
std::atomic_uint64_t element::m_nextId = 1;

std::string to_string(relationship_t r)
{
    switch (r) {
    case relationship_t::kNone:
        return "none";
    case relationship_t::kExtension:
        return "extension";
    case relationship_t::kComposition:
        return "composition";
    case relationship_t::kAggregation:
        return "aggregation";
    case relationship_t::kContainment:
        return "containment";
    case relationship_t::kOwnership:
        return "ownership";
    case relationship_t::kAssociation:
        return "association";
    case relationship_t::kInstantiation:
        return "instantiation";
    case relationship_t::kFriendship:
        return "frendship";
    case relationship_t::kDependency:
        return "dependency";
    default:
        return "invalid";
    }
}

//
// stylable_element
//
void stylable_element::set_style(const std::string &style) { style_ = style; }

std::string stylable_element::style() const { return style_; }

//
// decorated_element
//

bool decorated_element::skip() const
{
    for (auto d : decorators_)
        if (std::dynamic_pointer_cast<decorators::skip>(d))
            return true;

    return false;
}

bool decorated_element::skip_relationship() const
{
    for (auto d : decorators_)
        if (std::dynamic_pointer_cast<decorators::skip_relationship>(d))
            return true;

    return false;
}

std::pair<relationship_t, std::string> decorated_element::relationship() const
{
    for (auto &d : decorators_)
        if (std::dynamic_pointer_cast<decorators::association>(d))
            return {relationship_t::kAssociation,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};
        else if (std::dynamic_pointer_cast<decorators::aggregation>(d))
            return {relationship_t::kAggregation,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};
        else if (std::dynamic_pointer_cast<decorators::composition>(d))
            return {relationship_t::kComposition,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};

    return {relationship_t::kNone, ""};
}

std::string decorated_element::style_spec()
{
    for (auto d : decorators_)
        if (std::dynamic_pointer_cast<decorators::style>(d))
            return std::dynamic_pointer_cast<decorators::style>(d)->spec;

    return "";
}

const std::vector<std::shared_ptr<decorators::decorator>> &
decorated_element::decorators() const
{
    return decorators_;
}

void decorated_element::add_decorators(
    const std::vector<std::shared_ptr<decorators::decorator>> &decorators)
{
    for (auto d : decorators) {
        decorators_.push_back(d);
    }
}

//
// class_element
//
class_element::class_element(
    scope_t scope, const std::string &name, const std::string &type)
    : scope_{scope}
    , name_{name}
    , type_{type}
{
}

scope_t class_element::scope() const { return scope_; }

std::string class_element::name() const { return name_; }

std::string class_element::type() const { return type_; }

//
// class_member
//
class_member::class_member(
    scope_t scope, const std::string &name, const std::string &type)
    : class_element{scope, name, type}
{
}

bool class_member::is_relationship() const { return is_relationship_; }

void class_member::is_relationship(bool is_relationship)
{
    is_relationship_ = is_relationship;
}

bool class_member::is_static() const { return is_static_; }

void class_member::is_static(bool is_static) { is_static_ = is_static; }

//
// element
//

element::element(const std::vector<std::string> &using_namespaces)
    : using_namespaces_{using_namespaces}
    , m_id{m_nextId++}
{
}

std::string element::alias() const { return fmt::format("C_{:010}", m_id); }

void element::add_relationship(class_relationship &&cr)
{
    if (cr.destination.empty()) {
        LOG_WARN("Skipping relationship '{}' - {} - '{}' due empty "
                 "destination",
            cr.destination, to_string(cr.type), full_name(true));
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

//
// method_parameter
//
void method_parameter::set_type(const std::string &type) { type_ = type; }

std::string method_parameter::type() const { return type_; }

void method_parameter::set_name(const std::string &name) { name_ = name; }

std::string method_parameter::name() const { return name_; }

void method_parameter::set_default_value(const std::string &value)
{
    default_value_ = value;
}

std::string method_parameter::default_value() const { return default_value_; }

std::string method_parameter::to_string(
    const std::vector<std::string> &using_namespaces) const
{
    using namespace clanguml::util;
    auto t = ns_relative(using_namespaces, type());
    if (default_value().empty())
        return fmt::format("{} {}", t, name());

    return fmt::format("{} {} = {}", t, name(), default_value());
}

//
// class_method
//
class_method::class_method(
    scope_t scope, const std::string &name, const std::string &type)
    : class_element{scope, name, type}
{
}

bool class_method::is_pure_virtual() const { return is_pure_virtual_; }

void class_method::is_pure_virtual(bool is_pure_virtual)
{
    is_pure_virtual_ = is_pure_virtual;
}

bool class_method::is_virtual() const { return is_virtual_; }

void class_method::is_virtual(bool is_virtual) { is_virtual_ = is_virtual; }

bool class_method::is_const() const { return is_const_; }

void class_method::is_const(bool is_const) { is_const_ = is_const; }

bool class_method::is_defaulted() const { return is_defaulted_; }

void class_method::is_defaulted(bool is_defaulted)
{
    is_defaulted_ = is_defaulted;
}

bool class_method::is_static() const { return is_static_; }

void class_method::is_static(bool is_static) { is_static_ = is_static; }

const std::vector<method_parameter> &class_method::parameters() const
{
    return parameters_;
}

void class_method::add_parameter(method_parameter &&parameter)
{
    parameters_.emplace_back(std::move(parameter));
}

//
// class_parent
//
void class_parent::set_name(const std::string &name) { name_ = name; }

std::string class_parent::name() const { return name_; }

void class_parent::is_virtual(bool is_virtual) { is_virtual_ = is_virtual; }

bool class_parent::is_virtual() const { return is_virtual_; }

void class_parent::set_access(class_parent::access_t access)
{
    access_ = access;
}

class_parent::access_t class_parent::access() const { return access_; }

//
// class_relationship
//

bool operator==(const class_relationship &l, const class_relationship &r)
{
    return l.type == r.type && l.destination == r.destination &&
        l.label == r.label;
}

//
// class_template
//

bool operator==(const class_template &l, const class_template &r)
{
    return (l.name == r.name) && (l.type == r.type);
}

//
// type_alias
//
void type_alias::set_alias(const std::string &alias) { alias_ = alias; }

std::string type_alias::alias() const { return alias_; }

void type_alias::set_underlying_type(const std::string &type)
{
    underlying_type_ = type;
}

std::string type_alias::underlying_type() const { return underlying_type_; }

//
// class_
//
class_::class_(const std::vector<std::string> &using_namespaces)
    : element{using_namespaces}
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

void class_::add_member(class_member &&member)
{
    members_.emplace_back(std::move(member));
}

void class_::add_method(class_method &&method)
{
    methods_.emplace_back(std::move(method));
}

void class_::add_parent(class_parent &&parent)
{
    bases_.emplace_back(std::move(parent));
}

void class_::add_template(class_template &&tmplt)
{
    templates_.emplace_back(std::move(tmplt));
}

const std::vector<class_member> &class_::members() const { return members_; }

const std::vector<class_method> &class_::methods() const { return methods_; }

const std::vector<class_parent> &class_::parents() const { return bases_; }

const std::vector<class_template> &class_::templates() const
{
    return templates_;
}

void class_::set_base_template(const std::string &full_name)
{
    base_template_full_name_ = full_name;
}

std::string class_::base_template() const { return base_template_full_name_; }

bool operator==(const class_ &l, const class_ &r)
{
    return l.full_name() == r.full_name();
}

void class_::add_type_alias(type_alias &&ta)
{
    LOG_DBG("Adding class alias: {} -> {}", ta.alias(), ta.underlying_type());
    type_aliases_[ta.alias()] = std::move(ta);
}

std::string class_::full_name(bool relative) const
{
    using namespace clanguml::util;

    std::ostringstream ostr;
    if (relative)
        ostr << ns_relative(using_namespaces(), name());
    else
        ostr << name();

    if (!templates_.empty()) {
        std::vector<std::string> tnames;
        std::transform(templates_.cbegin(), templates_.cend(),
            std::back_inserter(tnames), [this](const auto &tmplt) {
                std::vector<std::string> res;

                if (!tmplt.type.empty())
                    res.push_back(ns_relative(using_namespaces(), tmplt.type));

                if (!tmplt.name.empty())
                    res.push_back(ns_relative(using_namespaces(), tmplt.name));

                if (!tmplt.default_value.empty()) {
                    res.push_back("=");
                    res.push_back(tmplt.default_value);
                }

                return fmt::format("{}", fmt::join(res, " "));
            });
        ostr << fmt::format("<{}>", fmt::join(tnames, ","));
    }

    return ostr.str();
}

bool class_::is_abstract() const
{
    // TODO check if all base abstract methods are overriden
    // with non-abstract methods
    return std::any_of(methods_.begin(), methods_.end(),
        [](const auto &method) { return method.is_pure_virtual(); });
}

//
// enum_
//
enum_::enum_(const std::vector<std::string> &using_namespaces)
    : element{using_namespaces}
{
}

bool operator==(const enum_ &l, const enum_ &r) { return l.name() == r.name(); }

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

//
// diagram
//
std::string diagram::name() const { return name_; }

void diagram::set_name(const std::string &name) { name_ = name; }

const std::vector<class_> diagram::classes() const { return classes_; }

const std::vector<enum_> diagram::enums() const { return enums_; }

bool diagram::has_class(const class_ &c) const
{
    return std::any_of(classes_.cbegin(), classes_.cend(),
        [&c](const auto &cc) { return cc.full_name() == c.full_name(); });
}

void diagram::add_type_alias(type_alias &&ta)
{
    LOG_DBG("Adding global alias: {} -> {}", ta.alias(), ta.underlying_type());

    type_aliases_[ta.alias()] = std::move(ta);
}

void diagram::add_class(class_ &&c)
{
    LOG_DBG("Adding class: {}, {}", c.name(), c.full_name());
    if (!has_class(c))
        classes_.emplace_back(std::move(c));
    else
        LOG_DBG("Class {} ({}) already in the model", c.name(), c.full_name());
}

void diagram::add_enum(enum_ &&e)
{
    LOG_DBG("Adding enum: {}", e.name());
    auto it = std::find(enums_.begin(), enums_.end(), e);
    if (it == enums_.end())
        enums_.emplace_back(std::move(e));
    else
        LOG_DBG("Enum {} already in the model", e.name());
}

std::string diagram::to_alias(const std::string &full_name) const
{
    LOG_DBG("Looking for alias for {}", full_name);

    for (const auto &c : classes_) {
        if (c.full_name() == full_name) {
            return c.alias();
        }
    }

    for (const auto &e : enums_) {
        if (e.full_name() == full_name) {
            return e.alias();
        }
    }

    throw error::uml_alias_missing(
        fmt::format("Missing alias for {}", full_name));
}

}
}
}
