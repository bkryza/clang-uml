/**
 * src/common/model/element.h
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

#include "decorated_element.h"
#include "namespace.h"
#include "relationship.h"
#include "source_location.h"
#include "util/util.h"

#include <inja/inja.hpp>

#include <atomic>
#include <exception>
#include <string>
#include <vector>

namespace clanguml::common::model {

class element : public decorated_element, public source_location {
public:
    element(const namespace_ &using_namespace);

    virtual ~element() = default;

    std::string alias() const;

    void set_name(const std::string &name) { name_ = name; }

    std::string name() const { return name_; }

    std::string name_and_ns() const
    {
        auto ns = ns_ | name();
        return ns.to_string();
    }

    void set_namespace(const namespace_ &ns) { ns_ = ns; }

    namespace_ get_namespace() const { return ns_; }

    namespace_ get_relative_namespace() const
    {
        return ns_.relative_to(using_namespace_);
    }

    virtual std::string full_name(bool relative) const { return name_and_ns(); }

    void set_using_namespaces(const namespace_ &un);

    const namespace_ &using_namespace() const;

    std::vector<relationship> &relationships();

    const std::vector<relationship> &relationships() const;

    void add_relationship(relationship &&cr);

    void append(const element &e);

    friend bool operator==(const element &l, const element &r);

    friend std::ostream &operator<<(std::ostream &out, const element &rhs);

    virtual inja::json context() const;

protected:
    const uint64_t m_id{0};

private:
    std::string name_;
    namespace_ ns_;
    namespace_ using_namespace_;
    std::vector<relationship> relationships_;
    type_safe::optional<source_location> location_;

    static std::atomic_uint64_t m_nextId;
};
}
