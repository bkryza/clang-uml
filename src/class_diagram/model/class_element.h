/**
 * @file src/class_diagram/model/class_element.h
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

#include "common/model/decorated_element.h"
#include "common/model/source_location.h"

#include <inja/inja.hpp>

#include <string>

namespace clanguml::class_diagram::model {

/**
 * @brief Base class for class elements (e.g. member or method).
 */
class class_element : public common::model::decorated_element,
                      public common::model::source_location {
public:
    class_element(
        common::model::access_t scope, std::string name, std::string type);

    ~class_element() override = default;

    /**
     * @brief Get elements access scope.
     *
     * @return Elements access scope.
     */
    common::model::access_t access() const;

    /**
     * @brief Get elements name.
     *
     * @return Elements name.
     */
    std::string name() const;

    /**
     * @brief Set elements name.
     *
     * @param name Elements name.
     */
    void set_name(const std::string &name);

    /**
     * @brief Get elements type as string.
     *
     * @return Elements type as string.
     */
    std::string type() const;

    /**
     * @brief Set elements type as string.
     *
     * @param type Elements type as string.
     */
    void set_type(const std::string &type);

    /**
     * @brief Get elements inja context in JSON.
     *
     * @return Context in JSON
     */
    virtual inja::json context() const;

private:
    common::model::access_t access_;
    std::string name_;
    std::string type_;
};

} // namespace clanguml::class_diagram::model
