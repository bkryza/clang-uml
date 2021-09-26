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

struct stylable_element {
    std::string style;
};

struct decorated_element {
    std::vector<std::shared_ptr<decorators::decorator>> decorators;

    bool skip() const;

    bool skip_relationship() const;

    std::pair<relationship_t, std::string> relationship() const;

    std::string style_spec();
};

struct class_element : public decorated_element {
    scope_t scope;
    std::string name;
    std::string type;
};

struct class_member : public class_element {
    bool is_relationship{false};
    bool is_static{false};
};

struct method_parameter : public decorated_element {
    std::string type;
    std::string name;
    std::string default_value;

    std::string to_string(
        const std::vector<std::string> &using_namespaces) const;
};

struct class_method : public class_element {
    std::vector<method_parameter> parameters;
    bool is_pure_virtual{false};
    bool is_virtual{false};
    bool is_const{false};
    bool is_defaulted{false};
    bool is_static{false};
};

struct class_parent {
    enum class access_t { kPublic, kProtected, kPrivate };
    std::string name;
    bool is_virtual{false};
    access_t access;
};

struct class_relationship : public decorated_element, public stylable_element {
    relationship_t type{relationship_t::kAssociation};
    std::string destination;
    std::string multiplicity_source;
    std::string multiplicity_destination;
    std::string label;
    scope_t scope{scope_t::kNone};

    friend bool operator==(
        const class_relationship &l, const class_relationship &r);
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

struct type_alias {
    std::string alias;
    std::string underlying_type;
};

class class_ : public element, public stylable_element {
public:
    class_(const std::vector<std::string> &using_namespaces);

    bool is_struct() const;
    void set_is_struct(bool is_struct);

    bool is_template() const;
    void set_is_template(bool is_template);

    bool is_template_instantiation() const;
    void set_is_template_instantiation(bool is_template_instantiation);

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
