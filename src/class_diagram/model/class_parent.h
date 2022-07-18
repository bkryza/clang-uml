/**
 * src/class_diagram/model/class_parent.h
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

#include "common/model/enums.h"
#include "common/types.h"

#include <string>

namespace clanguml::class_diagram::model {

class class_parent {
public:
    void set_name(const std::string &name);
    std::string name() const;

    clanguml::common::id_t id() const noexcept { return id_; }
    void set_id(clanguml::common::id_t id) { id_ = id; }

    void set_id(id_t id);

    void is_virtual(bool is_virtual);
    bool is_virtual() const;

    void set_access(common::model::access_t access);
    common::model::access_t access() const;

private:
    clanguml::common::id_t id_;
    std::string name_;
    bool is_virtual_{false};
    common::model::access_t access_;
};
}
