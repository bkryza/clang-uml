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

#include "diagram_element.h"
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

class element : public diagram_element {
public:
    element(namespace_ using_namespace);

    virtual ~element() = default;

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

    const namespace_ &path() const { return ns_; }

    std::string full_name(bool /*relative*/) const override
    {
        return name_and_ns();
    }

    virtual std::string full_name_no_ns() const { return name(); }

    void set_using_namespaces(const namespace_ &un);

    const namespace_ &using_namespace() const;

    friend bool operator==(const element &l, const element &r);

    friend std::ostream &operator<<(std::ostream &out, const element &rhs);

    inja::json context() const override;

private:
    namespace_ ns_;
    namespace_ using_namespace_;
};
} // namespace clanguml::common::model
