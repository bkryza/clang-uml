/**
 * @file src/common/model/relationship.h
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
#include "common/model/stylable_element.h"
#include "common/types.h"

#include <string>

namespace clanguml::common::model {

using clanguml::common::eid_t;

/**
 * @brief Class representing any relationship other than inheritance
 *
 * This class represents all kinds of relationships between diagram elements,
 * except for inheritance which are handled in a special way
 * (See @ref clanguml::class_diagram::model::class_parent).
 *
 * @embed{relationship_context_class.svg}
 */
class relationship : public common::model::decorated_element,
                     public common::model::stylable_element {
public:
    /**
     * Constructor.
     *
     * @param type Type of relationship
     * @param destination Id of the relationship target
     * @param access Access scope of the relationship
     * @param label Relationship label
     * @param multiplicity_source Multiplicity at the source
     * @param multiplicity_destination Multiplicity at the destination
     */
    relationship(relationship_t type, eid_t destination,
        access_t access = access_t::kPublic, std::string label = "",
        std::string multiplicity_source = "",
        std::string multiplicity_destination = "");

    ~relationship() override = default;

    /**
     * Set the type of relatinoship.
     *
     * @param type Type of relationship.
     */
    void set_type(relationship_t type) noexcept;

    /**
     * Get the type of relatinoship.
     *
     * @return Type of relationship.
     */
    relationship_t type() const noexcept;

    /**
     * Set id of the diagram element which is the target of this
     * relationship.
     *
     * @param destination Target element id.
     */
    void set_destination(eid_t destination);

    /**
     * Get the id of the target element of this relationship.
     *
     * @return Target element id.
     */
    eid_t destination() const;

    /**
     * Set the relationship multiplicity at the source.
     *
     * @param multiplicity_source Source multiplicity.
     */
    void set_multiplicity_source(const std::string &multiplicity_source);

    /**
     * Set the relationship multiplicity at the source.
     *
     * @return Source multiplicity.
     */
    std::string multiplicity_source() const;

    /**
     * Set the relationship multiplicity at the destination.
     *
     * @param multiplicity_destination Destination multiplicity.
     */
    void set_multiplicity_destination(
        const std::string &multiplicity_destination);

    /**
     * Set the relationship multiplicity at the destination.
     *
     * @return Destination multiplicity.
     */
    std::string multiplicity_destination() const;

    /**
     * Set relationship label.
     *
     * @param label Relationship label.
     */
    void set_label(const std::string &label);

    /**
     * Get the relationship label.
     *
     * @return Relationoship label.
     */
    std::string label() const;

    /**
     * Set the access scope for this relationship (e.g `public`)
     *
     * @param scope Access scope
     */
    void set_access(access_t scope) noexcept;

    /**
     * Get the relationship access scope (e.g. `public`).
     *
     * @return Access scope
     */
    access_t access() const noexcept;

    friend bool operator==(const relationship &l, const relationship &r);

private:
    relationship_t type_;
    eid_t destination_;
    std::string multiplicity_source_;
    std::string multiplicity_destination_;
    std::string label_;
    access_t access_;
};
} // namespace clanguml::common::model
