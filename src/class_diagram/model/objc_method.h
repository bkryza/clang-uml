/**
 * @file src/class_diagram/model/objc_method.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include "class_method_base.h"
#include "method_parameter.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

/**
 * @brief ObjC class method model.
 *
 * This includes both interface and protocol methods.
 */
class objc_method : public class_method_base {
public:
    /**
     * @brief Constructor.
     *
     * @param access Methods access scope (e.g. public)
     * @param name Methods name.
     * @param type Methods return type as string.
     */
    objc_method(common::model::access_t access, const std::string &name,
        const std::string &type);

    ~objc_method() override = default;

    /**
     * @brief Sets whether the protocol method is optional.
     *
     * @param io A boolean value indicating whether the method is optional.
     */
    void is_optional(bool io);

    /**
     * @brief Checks if the protocol method is optional.
     *
     * @return True if the method is optional, false otherwise.
     */
    bool is_optional() const;

private:
    bool is_optional_{false};
};
} // namespace clanguml::class_diagram::model
