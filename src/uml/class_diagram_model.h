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

#include "util/error.h"
#include "util/util.h"
#include "decorators.h"

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

class element {
public:
    element()
        : m_id{m_nextId++}
    {
    }
    std::string name;
    std::vector<std::string> namespace_;
    std::vector<std::shared_ptr<decorators::decorator>> decorators;

    std::string alias() const { return fmt::format("C_{:010}", m_id); }

protected:
    const uint64_t m_id{0};

private:
    static std::atomic_uint64_t m_nextId;
};

struct class_element {
    scope_t scope;
    std::string name;
    std::string type;
};

struct class_member : public class_element {
    bool is_relationship{false};
    bool is_static{false};
};

struct method_parameter {
    std::string type;
    std::string name;
    std::string default_value;

    std::string to_string(
        const std::vector<std::string> &using_namespaces) const
    {
        using namespace clanguml::util;
        auto t = ns_relative(using_namespaces, type);
        if (default_value.empty())
            return fmt::format("{} {}", t, name);

        return fmt::format("{} {} = {}", t, name, default_value);
    }
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

struct class_relationship {
    relationship_t type{relationship_t::kAssociation};
    std::string destination;
    std::string cardinality_source;
    std::string cardinality_destination;
    std::string label;
    scope_t scope{scope_t::kNone};

    friend bool operator==(
        const class_relationship &l, const class_relationship &r)
    {
        return l.type == r.type && l.destination == r.destination &&
            l.label == r.label;
    }
};

struct class_template {
    std::string name;
    std::string type;
    std::string default_value;
    bool is_variadic{false};

    friend bool operator==(const class_template &l, const class_template &r)
    {
        return (l.name == r.name) && (l.type == r.type);
    }
};

struct type_alias {
    std::string alias;
    std::string underlying_type;
};

class class_ : public element {
public:
    std::string usr;
    bool is_struct{false};
    bool is_template{false};
    bool is_template_instantiation{false};
    std::vector<class_member> members;
    std::vector<class_method> methods;
    std::vector<class_parent> bases;
    std::vector<std::string> inner_classes;
    std::vector<class_relationship> relationships;
    std::vector<class_template> templates;
    std::string base_template_usr;
    std::map<std::string, type_alias> type_aliases;

    friend bool operator==(const class_ &l, const class_ &r)
    {
        return (l.usr == r.usr) && (l.templates == r.templates);
    }

    void add_type_alias(type_alias &&ta)
    {
        LOG_DBG("Adding class alias: {} -> {}", ta.alias, ta.underlying_type);
        type_aliases[ta.alias] = std::move(ta);
    }

    void add_relationship(class_relationship &&cr)
    {
        if (cr.destination.empty()) {
            LOG_WARN(
                "Skipping relationship '{}' - {} - '{}' due empty destination",
                cr.destination, to_string(cr.type), usr);
            return;
        }

        auto it = std::find(relationships.begin(), relationships.end(), cr);
        if (it == relationships.end())
            relationships.emplace_back(std::move(cr));
    }

    std::string full_name(
        const std::vector<std::string> &using_namespaces) const
    {
        using namespace clanguml::util;

        std::ostringstream ostr;
        ostr << ns_relative(using_namespaces, name);

        if (!templates.empty()) {
            std::vector<std::string> tnames;
            std::transform(templates.cbegin(), templates.cend(),
                std::back_inserter(tnames),
                [&using_namespaces](const auto &tmplt) {
                    std::vector<std::string> res;

                    if (!tmplt.type.empty())
                        res.push_back(
                            ns_relative(using_namespaces, tmplt.type));

                    if (!tmplt.name.empty())
                        res.push_back(
                            ns_relative(using_namespaces, tmplt.name));

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

    bool is_abstract() const
    {
        // TODO check if all base abstract methods are overriden
        // with non-abstract methods
        return std::any_of(methods.begin(), methods.end(),
            [](const auto &method) { return method.is_pure_virtual; });
    }
};

struct enum_ : public element {
    std::vector<std::string> constants;
    std::vector<class_relationship> relationships;

    friend bool operator==(const enum_ &l, const enum_ &r)
    {
        return l.name == r.name;
    }

    std::string full_name(
        const std::vector<std::string> &using_namespaces) const
    {
        using namespace clanguml::util;

        std::ostringstream ostr;
        ostr << ns_relative(using_namespaces, name);

        return ostr.str();
    }
};

struct diagram {
    std::string name;
    std::vector<class_> classes;
    std::vector<enum_> enums;
    std::map<std::string, type_alias> type_aliases;

    bool has_class(const std::string &usr) const
    {
        return std::any_of(classes.cbegin(), classes.cend(),
            [&usr](const auto &c) { return c.usr == usr; });
    }

    void add_type_alias(type_alias &&ta)
    {
        LOG_DBG("Adding global alias: {} -> {}", ta.alias, ta.underlying_type);

        type_aliases[ta.alias] = std::move(ta);
    }

    void add_class(class_ &&c)
    {
        LOG_DBG("Adding class: {}, {}", c.name, c.usr);
        if (!has_class(c.usr))
            classes.emplace_back(std::move(c));
        else
            LOG_DBG("Class {} already in the model", c.name);
    }

    void add_enum(enum_ &&e)
    {
        LOG_DBG("Adding enum: {}", e.name);
        auto it = std::find(enums.begin(), enums.end(), e);
        if (it == enums.end())
            enums.emplace_back(std::move(e));
        else
            LOG_DBG("Enum {} already in the model", e.name);
    }

    std::string to_alias(const std::vector<std::string> &using_namespaces,
        const std::string &full_name) const
    {
        for (const auto &c : classes) {
            if (c.full_name(using_namespaces) == full_name) {
                return c.alias();
            }
        }

        for (const auto &e : enums) {
            if (e.full_name(using_namespaces) == full_name) {
                return e.alias();
            }
        }

        throw error::uml_alias_missing(
            fmt::format("Missing alias for {}", full_name));
    }

    std::string usr_to_name(const std::vector<std::string> &using_namespaces,
        const std::string &usr) const
    {
        if (usr.empty())
            throw std::runtime_error("Empty USR");

        for (const auto &c : classes) {
            if (c.usr == usr)
                return c.full_name(using_namespaces);
        }

        return "";
    }
};
}
}
}
