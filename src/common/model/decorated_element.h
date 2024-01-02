/**
 * @file src/common/model/decorated_element.h
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

#include "enums.h"

#include "decorators/decorators.h"
#include "inja/inja.hpp"

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace clanguml::common::model {

using comment_t = inja::json;

/**
 * @brief Base class for decorated diagram elements
 *
 * Decorators in `clang-uml` mean that custom `@uml{}` directives can be
 * applied to them in the code comments.
 *
 * @embed{decorated_element_hierarchy_class.svg}
 *
 * @see clanguml::decorators::decorator
 *
 */
class decorated_element {
public:
    virtual ~decorated_element() = default;

    /**
     * Whether this element should be skipped from the diagram.
     *
     * @return
     */
    bool skip() const;

    /**
     * Whether this relationship should be skipped from the diagram.
     *
     * @return
     */
    bool skip_relationship() const;

    /**
     * If this element is a member or a method, get relationship decorator
     * if any.
     *
     * @code
     *   /// @uml{aggregation[0..1:1..5]}
     *   std::vector<C> ccc;
     * @endcode
     *
     * @return Relationship specified as a decorator on class member.
     */
    std::pair<relationship_t, std::string> get_relationship() const;

    /**
     * Get stype specification for this element, if any.
     *
     * @code
     *   /// @uml{style[#back:lightgreen|yellow;header:blue/red]}
     *   class A { };
     * @endcode
     *
     * @return
     */
    std::string style_spec() const;

    /**
     * Get all decorators for this element.
     *
     * @return List of decorator pointers.
     */
    const std::vector<std::shared_ptr<decorators::decorator>> &
    decorators() const;

    /**
     * Add decorators to the element.
     *
     * @param decorators List of decorator pointers.
     */
    void add_decorators(
        const std::vector<std::shared_ptr<decorators::decorator>> &decorators);

    /**
     * Append decorators from another element.
     *
     * @param de Source element to copy decorators from.
     */
    void append(const decorated_element &de);

    /**
     * Get entire comment model for this element.
     *
     * @return Comment model.
     */
    std::optional<comment_t> comment() const;

    /**
     * Set comment model for this element.
     *
     * Comment model is currently a JSON object.
     *
     * @param c Comment model.
     */
    void set_comment(const comment_t &c);

    /**
     * Return Doxygen HTML documentation link for the element.
     *
     * @return Element context.
     */
    virtual std::optional<std::string> doxygen_link() const;

private:
    std::vector<std::shared_ptr<decorators::decorator>> decorators_;
    std::optional<comment_t> comment_;
};

} // namespace clanguml::common::model
