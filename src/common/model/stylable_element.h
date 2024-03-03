/**
 * @file src/common/model/stylable_element.h
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

#include <optional>
#include <string>

namespace clanguml::common::model {

/**
 * @brief Diagram elements to which style can be applied.
 *
 * @embed{stylable_element_hierarchy_class.svg}
 */
class stylable_element {
public:
    /**
     * Set style.
     *
     * @param style Style specification
     */
    void set_style(const std::string &style);

    /**
     * Get style
     *
     * @return Style specification
     */
    std::optional<std::string> style() const;

private:
    std::optional<std::string> style_;
};

} // namespace clanguml::common::model
