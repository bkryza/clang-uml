/**
 * @file src/class_diagram/model/class_parent.h
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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

#include "common/clang_utils.h"
#include "common/model/enums.h"
#include "common/types.h"

#include <string>

namespace clanguml::class_diagram::model {

using clanguml::common::eid_t;

/**
 * @brief Class parent relationship model.
 *
 * @todo Consider refactoring this class to a regular relationship.
 */
class class_parent {
public:
    class_parent() = default;

    class_parent(const std::string &name)
    {
        set_name(name);
        set_id(common::to_id(name));
    }

    /**
     * @brief Set the fully qualified name of class parent.
     *
     * @param name Fully qualified name of the parent class.
     */
    void set_name(const std::string &name);

    /**
     * @brief Get the fully qualified name of class parent.
     *
     * @return Fully qualified name of the parent class.
     */
    std::string name() const;

    /**
     * @brief Set the id of class parent.
     *
     * @param id Id of the parent class.
     */
    void set_id(eid_t id);

    /**
     * @brief Get the id of class parent.
     *
     * @return Id of the parent class.
     */
    eid_t id() const noexcept;

    /**
     * @brief Set whether the parent is virtual.
     *
     * @param is_virtual True if the parent is virtual
     */
    void is_virtual(bool is_virtual);

    /**
     * @brief Get whether the parent is virtual.
     *
     * @return True, if the parent is virtual
     */
    bool is_virtual() const;

    /**
     * @brief Set the parents access scope
     *
     * @param access Parents access scope
     */
    void set_access(common::model::access_t access);

    /**
     * @brief Get parents access scope.
     *
     * @return Parents access scope.
     */
    common::model::access_t access() const;

private:
    eid_t id_{};
    std::string name_;
    bool is_virtual_{false};
    common::model::access_t access_{common::model::access_t::kPublic};
};
} // namespace clanguml::class_diagram::model
