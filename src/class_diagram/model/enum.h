/**
 * @file src/class_diagram/model/enum.h
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

#include "class.h"

#include <string>
#include <vector>

namespace clanguml::class_diagram::model {

/*
 * @brief Diagram element representing an enum.
 */
class enum_ : public common::model::element,
              public common::model::stylable_element {
public:
    enum_(const common::model::namespace_ &using_namespaces);

    enum_(const enum_ &) = delete;
    enum_(enum_ &&) = delete;
    enum_ &operator=(const enum_ &) = delete;
    enum_ &operator=(enum_ &&) = delete;

    std::string type_name() const override { return "enum"; }

    friend bool operator==(const enum_ &l, const enum_ &r);

    std::string full_name(bool relative = true) const override;

    /**
     * @brief Get the enums constants.
     *
     * @return Enums constants names list.
     */
    std::vector<std::string> &constants();

    /**
     * @brief Get the enums constants.
     *
     * @return Enums constants names list.
     */
    const std::vector<std::string> &constants() const;

    /**
     * @brief Get Doxygen link to documentation page for this element.
     *
     * @return Doxygen link for this element.
     */
    std::optional<std::string> doxygen_link() const override;

private:
    std::vector<std::string> constants_;
};

} // namespace clanguml::class_diagram::model
