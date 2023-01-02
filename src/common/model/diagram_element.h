/**
 * src/common/model/diagram_element.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

#include "decorated_element.h"
#include "relationship.h"
#include "source_location.h"
#include "util/util.h"

#include <inja/inja.hpp>

#include <atomic>
#include <exception>
#include <string>
#include <vector>

namespace clanguml::common::model {

class diagram_element : public decorated_element, public source_location {
public:
    using id_t = int64_t;

    diagram_element();

    virtual ~diagram_element() = default;

    id_t id() const;

    void set_id(id_t id);

    virtual std::string alias() const;

    void set_name(const std::string &name) { name_ = name; }

    std::string name() const { return name_; }

    virtual std::string type_name() const { return "__undefined__"; };

    virtual std::string full_name(bool /*relative*/) const { return name(); }

    std::vector<relationship> &relationships();

    const std::vector<relationship> &relationships() const;

    void add_relationship(relationship &&cr);

    void append(const decorated_element &e);

    friend bool operator==(const diagram_element &l, const diagram_element &r);

    friend std::ostream &operator<<(
        std::ostream &out, const diagram_element &rhs);

    virtual inja::json context() const;

    bool is_nested() const;

    void nested(bool nested);

    bool complete() const;

    void complete(bool completed);

private:
    id_t id_{0};
    std::string name_;
    std::vector<relationship> relationships_;
    bool nested_{false};
    bool complete_{false};
};
} // namespace clanguml::common::model
