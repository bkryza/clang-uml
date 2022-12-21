/**
 * src/common/model/diagram.h
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

#include "diagram_element.h"
#include "enums.h"
#include "namespace.h"
#include "source_file.h"

#include <memory>
#include <string>

namespace clanguml::common::model {

class diagram_filter;
class element;
class relationship;

class diagram {
public:
    diagram();

    virtual ~diagram();

    virtual diagram_t type() const = 0;

    virtual common::optional_ref<clanguml::common::model::diagram_element> get(
        const std::string &full_name) const = 0;

    virtual common::optional_ref<clanguml::common::model::diagram_element> get(
        diagram_element::id_t id) const = 0;

    /// \brief Find element in diagram which can have full name or be
    ///        relative to ns
    common::optional_ref<clanguml::common::model::diagram_element>
    get_with_namespace(const std::string &name, const namespace_ &ns) const;

    diagram(const diagram &) = delete;
    diagram(diagram &&) noexcept;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) noexcept;

    void set_name(const std::string &name);
    std::string name() const;

    void set_filter(std::unique_ptr<diagram_filter> filter);

    void set_complete(bool complete);
    bool complete() const;

    // TODO: refactor to a template method
    bool should_include(const element &e) const;
    bool should_include(const std::string &e) const;
    bool should_include(const source_file &path) const;
    bool should_include(relationship r) const;
    bool should_include(relationship_t r) const;
    bool should_include(access_t s) const;

    virtual bool has_element(const diagram_element::id_t id) const
    {
        return false;
    }

    virtual bool should_include(
        const namespace_ &ns, const std::string &name) const;

    virtual inja::json context() const = 0;

private:
    std::string name_;
    std::unique_ptr<diagram_filter> filter_;
    bool complete_{false};
};

template <typename DiagramT> bool check_diagram_type(diagram_t t);
} // namespace clanguml::common::model
