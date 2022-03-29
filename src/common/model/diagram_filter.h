/**
 * src/common/model/entity_filter.h
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
#pragma once

#include "class_diagram/model/diagram.h"
#include "common/model/diagram.h"
#include "common/model/element.h"
#include "common/model/enums.h"
#include "common/model/namespace.h"
#include "config/config.h"
#include "cx/util.h"
#include "diagram.h"

namespace clanguml::common::model {

enum filter_t { kInclusive, kExclusive };

class filter_visitor {
public:
    filter_visitor(filter_t type)
        : type_{type}
    {
    }

    virtual std::optional<bool> match(
        const diagram &d, const common::model::element &e) const;

    virtual std::optional<bool> match(
        const diagram &d, const common::model::relationship_t &r) const;

    virtual std::optional<bool> match(
        const diagram &d, const common::model::scope_t &r) const;

    virtual std::optional<bool> match(
        const diagram &d, const common::model::namespace_ &ns) const;

    bool is_inclusive() const;
    bool is_exclusive() const;

    filter_t type() const;

private:
    filter_t type_;
};

struct namespace_filter : public filter_visitor {
    namespace_filter(filter_t type, std::vector<namespace_> namespaces);

    std::optional<bool> match(
        const diagram &d, const namespace_ &ns) const override;

    std::optional<bool> match(
        const diagram &d, const element &e) const override;

private:
    std::vector<namespace_> namespaces_;
};

struct element_filter : public filter_visitor {
    element_filter(filter_t type, std::vector<std::string> elements);

    std::optional<bool> match(
        const diagram &d, const element &e) const override;

private:
    std::vector<std::string> elements_;
};

struct subclass_filter : public filter_visitor {
    subclass_filter(filter_t type, std::vector<std::string> roots);

    std::optional<bool> match(
        const diagram &d, const element &e) const override;

private:
    std::vector<std::string> roots_;
};

struct relationship_filter : public filter_visitor {
    relationship_filter(
        filter_t type, std::vector<relationship_t> relationships);

    std::optional<bool> match(
        const diagram &d, const relationship_t &r) const override;

private:
    std::vector<relationship_t> relationships_;
};

struct scope_filter : public filter_visitor {
    scope_filter(filter_t type, std::vector<scope_t> scopes);

    std::optional<bool> match(
        const diagram &d, const scope_t &s) const override;

private:
    std::vector<scope_t> scopes_;
};

struct context_filter : public filter_visitor {
    context_filter(filter_t type, std::vector<std::string> context);

    std::optional<bool> match(
        const diagram &d, const element &r) const override;

private:
    std::vector<std::string> context_;
};

class diagram_filter {
public:
    diagram_filter(const common::model::diagram &d, const config::diagram &c);

    void add_inclusive_filter(std::unique_ptr<filter_visitor> fv);

    void add_exclusive_filter(std::unique_ptr<filter_visitor> fv);

    bool should_include(namespace_ ns, const std::string &name) const;

    template <typename T> bool should_include(const T &e) const
    {
        bool exc = std::any_of(
            exclusive_.begin(), exclusive_.end(), [this, &e](const auto &ex) {
                auto m = ex->match(diagram_, e);
                // Return true if a filter is defined for specific element
                // and it's a match
                return m.has_value() && m.value();
            });
        if (exc)
            return false;

        bool inc = std::all_of(
            inclusive_.begin(), inclusive_.end(), [this, &e](const auto &in) {
                auto m = in->match(diagram_, e);
                // Return true if a filter is undefined for specific element
                // or it's a match
                return !m.has_value() || m.value();
            });

        if (inc)
            return true;

        return false;
    }

private:
    void init_filters(const config::diagram &c);

    std::vector<std::unique_ptr<filter_visitor>> inclusive_;
    std::vector<std::unique_ptr<filter_visitor>> exclusive_;

    const common::model::diagram &diagram_;
};

template <>
bool diagram_filter::should_include<std::string>(const std::string &name) const;
}