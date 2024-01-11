/**
 * @file src/common/generators/nested_element_stack.h
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

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace clanguml::common::generators {

/**
 * This is a helper class for generating nested groups of elements
 * in the diagrams, e.g. PlantUML `together` option.
 *
 * @tparam T Type of stack elements
 */
template <typename T> class nested_element_stack {
public:
    nested_element_stack(bool is_flat)
        : is_flat_{is_flat}
    {
        current_level_groups_.push_back({});
    }

    /**
     * Switch to next level in the element stack
     */
    void enter()
    {
        if (!is_flat_)
            current_level_++;

        current_level_groups_.push_back({});
    }

    /**
     * Switch to previous level in the element stack
     */
    void leave()
    {
        if (!is_flat_)
            current_level_--;

        current_level_groups_.pop_back();
    }

    /**
     * Add element pointer to a specified group at the current level
     */
    void group_together(const std::string &group_name, T *e)
    {
        current_level_groups_[current_level_][group_name].push_back(e);
    }

    /**
     * Get map of element groups at the current level.
     *
     * @return Reference to element groups.
     */
    const std::map<std::string, std::vector<T *>> &get_current_groups()
    {
        return current_level_groups_.at(current_level_);
    }

    /**
     * Get element group by name - the group must exist at the current level.
     *
     * @param group_name Element group name
     * @return
     */
    const std::vector<T *> &get_group(const std::string &group_name)
    {
        return get_current_groups().at(group_name);
    }

private:
    bool is_flat_;

    uint32_t current_level_{0};

    std::vector<std::map<std::string, std::vector<T *>>> current_level_groups_;
};

} // namespace clanguml::common::generators