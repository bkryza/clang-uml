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
#include "relationship.h"
#include "util/util.h"

#include <atomic>
#include <exception>
#include <string>
#include <vector>

namespace clanguml::common::model {

class element : public decorated_element {
public:
    element(const std::vector<std::string> &using_namespaces);

    virtual ~element() = default;

    std::string alias() const;

    void set_name(const std::string &name) { name_ = name; }

    std::string name() const { return name_; }

    std::string name_and_ns() const
    {
        auto ns = namespace_;
        ns.push_back(name());
        return util::join(ns, "::");
    }

    void set_namespace(const std::vector<std::string> &ns) { namespace_ = ns; }

    std::vector<std::string> get_namespace() const { return namespace_; }

    std::vector<std::string> get_relative_namespace() const
    {
        auto relative_ns = namespace_;
        util::remove_prefix(relative_ns, using_namespaces_);
        return relative_ns;
    }

    virtual std::string full_name(bool relative) const { return name(); }

    void set_using_namespaces(const std::vector<std::string> &un);

    const std::vector<std::string> &using_namespaces() const;

    std::vector<relationship> &relationships();

    const std::vector<relationship> &relationships() const;

    void add_relationship(relationship &&cr);

    void append(const element &e);

    friend bool operator==(const element &l, const element &r);

    friend std::ostream &operator<<(std::ostream &out, const element &rhs);

protected:
    const uint64_t m_id{0};

private:
    std::string name_;
    std::vector<std::string> namespace_;
    std::vector<std::string> using_namespaces_;
    std::vector<relationship> relationships_;

    static std::atomic_uint64_t m_nextId;
};
}
