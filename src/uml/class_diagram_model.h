/**
 * src/uml/class_diagram_model.h
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
#pragma once

#include "decorators.h"
#include "util/error.h"
#include "util/util.h"

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

namespace clanguml {
namespace model {
namespace class_diagram {

enum class scope_t { kPublic, kProtected, kPrivate, kNone };

enum class relationship_t {
    kNone,
    kExtension,
    kComposition,
    kAggregation,
    kContainment,
    kOwnership,
    kAssociation,
    kInstantiation,
    kFriendship,
    kDependency
};

std::string to_string(relationship_t r);

class stylable_element {
public:
    void set_style(const std::string &style);
    std::string style() const;

private:
    std::string style_;
};

class decorated_element {
public:
    bool skip() const;

    bool skip_relationship() const;

    std::pair<relationship_t, std::string> relationship() const;

    std::string style_spec();

    const std::vector<std::shared_ptr<decorators::decorator>> &
    decorators() const;

    void add_decorators(
        const std::vector<std::shared_ptr<decorators::decorator>> &decorators);

private:
    std::vector<std::shared_ptr<decorators::decorator>> decorators_;
};

class class_element : public decorated_element {
public:
    class_element(
        scope_t scope, const std::string &name, const std::string &type);

    scope_t scope() const;
    std::string name() const;
    std::string type() const;

private:
    scope_t scope_;
    std::string name_;
    std::string type_;
};

class class_member : public class_element {
public:
    class_member(
        scope_t scope, const std::string &name, const std::string &type);

    bool is_relationship() const;
    void is_relationship(bool is_relationship);

    bool is_static() const;
    void is_static(bool is_static);

private:
    bool is_relationship_{false};
    bool is_static_{false};
};

class method_parameter : public decorated_element {
public:
    void set_type(const std::string &type);
    std::string type() const;

    void set_name(const std::string &name);
    std::string name() const;

    void set_default_value(const std::string &value);
    std::string default_value() const;

    std::string to_string(
        const std::vector<std::string> &using_namespaces) const;

private:
    std::string type_;
    std::string name_;
    std::string default_value_;
};

class class_method : public class_element {
public:
    class_method(
        scope_t scope, const std::string &name, const std::string &type);

    bool is_pure_virtual() const;
    void is_pure_virtual(bool is_pure_virtual);

    bool is_virtual() const;
    void is_virtual(bool is_virtual);

    bool is_const() const;
    void is_const(bool is_const);

    bool is_defaulted() const;
    void is_defaulted(bool is_defaulted);

    bool is_static() const;
    void is_static(bool is_static);

    const std::vector<method_parameter> &parameters() const;
    void add_parameter(method_parameter &&parameter);

private:
    std::vector<method_parameter> parameters_;
    bool is_pure_virtual_{false};
    bool is_virtual_{false};
    bool is_const_{false};
    bool is_defaulted_{false};
    bool is_static_{false};
};

class class_parent {
public:
    enum class access_t { kPublic, kProtected, kPrivate };

    void set_name(const std::string &name);
    std::string name() const;

    void is_virtual(bool is_virtual);
    bool is_virtual() const;

    void set_access(access_t access);
    access_t access() const;

private:
    std::string name_;
    bool is_virtual_{false};
    access_t access_;
};

class class_relationship : public decorated_element, public stylable_element {
public:
    class_relationship(relationship_t type, const std::string &destination,
        scope_t scope = scope_t::kNone, const std::string &label = "",
        const std::string &multiplicity_source = "",
        const std::string &multiplicity_destination = "");

    virtual ~class_relationship() = default;

    void set_type(relationship_t type) noexcept;
    relationship_t type() const noexcept;

    void set_destination(const std::string &destination);
    std::string destination() const;

    void set_multiplicity_source(const std::string &multiplicity_source);
    std::string multiplicity_source() const;

    void set_multiplicity_destination(
        const std::string &multiplicity_destination);
    std::string multiplicity_destination() const;

    void set_label(const std::string &label);
    std::string label() const;

    void set_scope(scope_t scope) noexcept;
    scope_t scope() const noexcept;

    friend bool operator==(
        const class_relationship &l, const class_relationship &r);

private:
    relationship_t type_{relationship_t::kAssociation};
    std::string destination_;
    std::string multiplicity_source_;
    std::string multiplicity_destination_;
    std::string label_;
    scope_t scope_{scope_t::kNone};
};

struct class_template {
    std::string name;
    std::string type;
    std::string default_value;
    bool is_variadic{false};

    friend bool operator==(const class_template &l, const class_template &r);
};

class element : public decorated_element {
public:
    element(const std::vector<std::string> &using_namespaces);

    std::string alias() const;

    void set_name(const std::string &name) { name_ = name; }

    std::string name() const { return name_; }

    void set_namespace(const std::vector<std::string> &ns) { namespace_ = ns; }

    std::vector<std::string> get_namespace() const { return namespace_; }

    virtual std::string full_name(bool relative) const { return name(); }

    void set_using_namespaces(const std::vector<std::string> &un);

    const std::vector<std::string> &using_namespaces() const;

    std::vector<class_relationship> &relationships();

    const std::vector<class_relationship> &relationships() const;

    void add_relationship(class_relationship &&cr);

protected:
    const uint64_t m_id{0};

private:
    std::string name_;
    std::vector<std::string> namespace_;
    std::vector<std::string> using_namespaces_;
    std::vector<class_relationship> relationships_;

    static std::atomic_uint64_t m_nextId;
};

class type_alias {
public:
    void set_alias(const std::string &alias);
    std::string alias() const;

    void set_underlying_type(const std::string &type);
    std::string underlying_type() const;

private:
    std::string alias_;
    std::string underlying_type_;
};

class class_ : public element, public stylable_element {
public:
    class_(const std::vector<std::string> &using_namespaces);

    bool is_struct() const;
    void is_struct(bool is_struct);

    bool is_template() const;
    void is_template(bool is_template);

    bool is_template_instantiation() const;
    void is_template_instantiation(bool is_template_instantiation);

    void add_member(class_member &&member);
    void add_method(class_method &&method);
    void add_parent(class_parent &&parent);
    void add_template(class_template &&tmplt);

    const std::vector<class_member> &members() const;
    const std::vector<class_method> &methods() const;
    const std::vector<class_parent> &parents() const;
    const std::vector<class_template> &templates() const;

    void set_base_template(const std::string &full_name);
    std::string base_template() const;

    friend bool operator==(const class_ &l, const class_ &r);

    void add_type_alias(type_alias &&ta);

    std::string full_name(bool relative = true) const override;

    bool is_abstract() const;

private:
    bool is_struct_{false};
    bool is_template_{false};
    bool is_template_instantiation_{false};
    std::vector<class_member> members_;
    std::vector<class_method> methods_;
    std::vector<class_parent> bases_;
    std::vector<class_template> templates_;
    std::string base_template_full_name_;
    std::map<std::string, type_alias> type_aliases_;

    std::string full_name_;
};

struct enum_ : public element, public stylable_element {
public:
    enum_(const std::vector<std::string> &using_namespaces);

    friend bool operator==(const enum_ &l, const enum_ &r);

    std::string full_name(bool relative = true) const override;

    std::vector<std::string> &constants();

    const std::vector<std::string> &constants() const;

private:
    std::vector<std::string> constants_;
};

class diagram {
public:
    std::string name() const;

    void set_name(const std::string &name);

    const std::vector<class_> classes() const;

    const std::vector<enum_> enums() const;

    bool has_class(const class_ &c) const;

    void add_type_alias(type_alias &&ta);

    void add_class(class_ &&c);

    void add_enum(enum_ &&e);

    std::string to_alias(const std::string &full_name) const;

private:
    std::string name_;
    std::vector<class_> classes_;
    std::vector<enum_> enums_;
    std::map<std::string, type_alias> type_aliases_;
};
}
}
}
