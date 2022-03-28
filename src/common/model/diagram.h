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

#include "enums.h"

#include <memory>
#include <string>

namespace clanguml::common::model {

class diagram_filter;
class namespace_;
class element;
class relationship;

class diagram {
public:
    diagram();
    virtual ~diagram();

    diagram(const diagram &) = delete;
    diagram(diagram &&);
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&);

    void set_name(const std::string &name);
    std::string name() const;

    void set_filter(std::unique_ptr<diagram_filter> filter);

    void set_complete(bool complete);
    bool complete() const;

    // TODO: refactor to a template method
    bool should_include(const element &e) const;
    bool should_include(const std::string &e) const;
    bool should_include(const relationship r) const;
    bool should_include(const relationship_t r) const;
    bool should_include(const scope_t s) const;
    bool should_include(const namespace_ &ns, const std::string &name) const;

private:
    std::string name_;
    std::unique_ptr<diagram_filter> filter_;
    bool complete_{false};
};

}
