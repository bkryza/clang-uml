/**
 * src/common/model/diagram_element.h
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

#include "decorated_element.h"
#include "relationship.h"
#include "source_location.h"
#include "util/memoized.h"
#include "util/util.h"

#include <inja/inja.hpp>

#include <atomic>
#include <exception>
#include <set>
#include <string>
#include <vector>

namespace clanguml::common::model {

class diagram_filter;

struct full_name_tag_t { };
struct name_and_ns_tag { };

/**
 * @brief Base class for standalone diagram elements.
 *
 * This is a base cass of any standalone elements such as classes, structs,
 * concepts, packages and so on participants and so on.
 */
class diagram_element
    : public decorated_element,
      public source_location,
      public util::memoized<full_name_tag_t, std::string, bool>,
      public util::memoized<name_and_ns_tag, std::string> {
public:
    diagram_element();

    ~diagram_element() override = default;

    /**
     * @brief Returns diagram element id.
     *
     * Each element in the diagram is uniquely identified by id. The id
     * is currently calculated from the full string representation of the
     * element, in order to be uniquely identifiable among multiple translation
     * units.
     *
     * @return Elements id.
     */
    const eid_t &id() const;

    /**
     * Set elements id.
     *
     * @param id Elements id.
     */
    void set_id(eid_t id);

    /**
     * Get elements parent package id.
     *
     * @return Parent package id if element is nested.
     */
    std::optional<eid_t> parent_element_id() const;

    /**
     * Set elements parent package id.
     *
     * @param id Id of parent package.
     */
    void set_parent_element_id(eid_t id);

    /**
     * @brief Return elements' diagram alias.
     *
     * @todo This is a PlantUML specific method - it shouldn't be here.
     *
     * @return PlantUML's diagram element alias.
     */
    virtual std::string alias() const;

    /**
     * Set diagram elements name.
     *
     * @param name Elements name.
     */
    void set_name(const std::string &name)
    {
        util::memoized<name_and_ns_tag, std::string>::invalidate();
        name_ = name;
    }

    /**
     * Set diagram elements name for nested elements.
     *
     * @param parent Parents name.
     * @param name Elements name.
     */
    void set_name(const std::string &parent, const std::string &name)
    {
        set_name(fmt::format("{}##{}", parent, name));
    }

    /**
     * Return diagram element name.
     *
     * @return Diagram element name.
     */
    std::string name() const { return name_; }

    /**
     * Return the type name of the diagram element.
     *
     * @return Diagrams element type name.
     */
    virtual std::string type_name() const { return "__undefined__"; };

    /**
     * @brief Return the elements fully qualified name.
     *
     * This method should be implemented in each subclass, and ensure that
     * for instance it includes fully qualified namespace, template params, etc.
     *
     * @return Full elements name.
     */
    std::string full_name(bool relative) const
    {
        return util::memoized<full_name_tag_t, std::string, bool>::memoize(
            complete(),
            [this](bool relative) { return full_name_impl(relative); },
            relative);
    }

    /**
     * Return all relationships outgoing from this element.
     *
     * @return List of relationships.
     */
    std::vector<relationship> &relationships();

    /**
     * Return all relationships outgoing from this element.
     *
     * @return List of relationships.
     */
    const std::vector<relationship> &relationships() const;

    /**
     * Add relationships, whose source is this element.
     *
     * @param cr Relationship to another diagram element.
     */
    void add_relationship(relationship &&cr);

    /**
     * Add element to the diagram.
     *
     * @param e Diagram element.
     */
    void append(const decorated_element &e);

    friend bool operator==(const diagram_element &l, const diagram_element &r);

    friend std::ostream &operator<<(
        std::ostream &out, const diagram_element &rhs);

    /**
     * Whether this element is nested in another element.
     *
     * @return
     */
    bool is_nested() const;

    /**
     * Set element's nested status.
     *
     * @param nested
     */
    void nested(bool nested);

    /**
     * Returns the diagrams completion status.
     *
     * @return Whether the diagram is complete.
     */
    bool complete() const;

    /**
     * Set the diagrams completion status.
     *
     * @param completed
     */
    void complete(bool completed);

    /**
     * Due to the fact that a relationship to the same element can be added
     * once with local TU id and other time with global id, the relationship
     * set can contain duplicates.
     */
    void remove_duplicate_relationships();

    virtual void apply_filter(
        const diagram_filter &filter, const std::set<eid_t> &removed);

    bool root_prefix() const { return false; }

protected:
    virtual std::string full_name_impl(bool /*relative*/) const
    {
        return name();
    }

private:
    eid_t id_{};
    std::optional<eid_t> parent_element_id_{};
    std::string name_;
    std::vector<relationship> relationships_;
    bool nested_{false};
    bool complete_{false};
};
} // namespace clanguml::common::model

template <typename T>
struct fmt::formatter<T,
    std::enable_if_t<
        std::is_base_of_v<clanguml::common::model::diagram_element, T>, char>>
    : fmt::formatter<std::string> {
    auto format(const clanguml::common::model::diagram_element &a,
        format_context &ctx) const
    {
        return formatter<std::string>::format(a.full_name(false), ctx);
    }
};