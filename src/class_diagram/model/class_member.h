/**
 * @file src/class_diagram/model/class_member.h
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

#include "class_element.h"

#include <string>

namespace clanguml::class_diagram::model {

/**
 * @brief Class member model.
 */
class class_member : public class_element {
public:
    /**
     * @brief Constructor.
     *
     * @param access Members access scope (e.g. public)
     * @param name Members name.
     * @param type Members type as string.
     */
    class_member(common::model::access_t access, const std::string &name,
        const std::string &type);

    ~class_member() override = default;

    /**
     * @brief Whether the member is static.
     *
     * @return True, if the member is static.
     */
    bool is_static() const;

    /**
     * @brief Set members static status.
     *
     * @param is_static True, if the member is static.
     */
    void is_static(bool is_static);

    /**
     * @brief Set members destination multiplicity.
     *
     * @param m Optional multiplicity value
     */
    void set_destination_multiplicity(std::optional<size_t> m);

    /**
     * @brief Get members destination multiplicity.
     *
     * @return Optional multiplicity value
     */
    std::optional<size_t> destination_multiplicity() const;

private:
    bool is_static_{false};
    std::optional<size_t> destination_multiplicity_{};
};

} // namespace clanguml::class_diagram::model
