/**
 * @file src/common/model/diagram.h
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

#include "diagram_element.h"
#include "enums.h"
#include "namespace.h"
#include "source_file.h"

#include <memory>
#include <string>

namespace clanguml::common::model {

class diagram_filter;
class element;
class relationship;

/**
 * @brief Base class for all diagram models
 *
 * @embed{diagram_hierarchy_class.svg}
 */
class diagram {
public:
    diagram();

    diagram(const diagram &) = delete;
    diagram(diagram && /*unused*/) noexcept;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram && /*unused*/) noexcept;

    virtual ~diagram();

    /**
     * @brief Return type of the diagram.
     *
     * @return Type of diagram
     */
    virtual diagram_t type() const = 0;

    /**
     * Return optional reference to a diagram_element by name.
     *
     * @param full_name Fully qualified name of a diagram element.
     * @return Optional reference to a diagram element.
     */
    virtual opt_ref<clanguml::common::model::diagram_element> get(
        const std::string &full_name) const = 0;

    /**
     * Return optional reference to a diagram_element by id.
     *
     * @param id Id of a diagram element.
     * @return Optional reference to a diagram element.
     */
    virtual common::optional_ref<clanguml::common::model::diagram_element> get(
        eid_t id) const = 0;

    /**
     * Return optional reference to a diagram_element by name and namespace.
     *
     * @param name Name of the diagram element (e.g. a class name)
     * @param ns Namespace of the element.
     * @return Optional reference to a diagram element.
     */
    virtual common::optional_ref<clanguml::common::model::diagram_element>
    get_with_namespace(const std::string &name, const namespace_ &ns) const;

    /**
     * Set diagram name.
     *
     * @param name Name of the diagram.
     */
    void set_name(const std::string &name);

    /**
     * Return the name of the diagram.
     *
     * @return Name of the diagram.
     */
    std::string name() const;

    /**
     * Set diagram filter for this diagram.
     *
     * @param filter diagram_filter instance
     *
     * @see clanguml::common::model::diagram_filter
     */
    void set_filter(std::unique_ptr<diagram_filter> filter);

    /**
     * Get diagram filter
     *
     * @return Reference to the diagrams element filter
     */
    const diagram_filter &filter() const { return *filter_; }

    /**
     * @brief Set diagram in a complete state.
     *
     * This must be called after the diagram's 'translation_unit_visitor' has
     * completed for all translation units, in order to apply filters which can
     * only work after the diagram is complete.
     *
     * @param complete Status of diagram visitor completion.
     */
    void set_complete(bool complete);

    /**
     * @brief Whether the diagram is complete.
     *
     * This flag is set to true, when all translation units for this diagram
     * have been visited.
     *
     * @return Diagram completion status.
     */
    bool complete() const;

    /**
     * @brief Once the diagram is complete, run any final processing.
     *
     * This method should be overriden by specific diagram models to do some
     * final tasks like cleaning up the model (e.g. some filters only work
     * on completed diagrams).
     */
    virtual void finalize();

    // TODO: refactor to a template method
    bool should_include(const element &e) const;
    bool should_include(const namespace_ &ns) const;
    bool should_include(const source_file &path) const;
    bool should_include(relationship r) const;
    bool should_include(relationship_t r) const;
    bool should_include(access_t s) const;
    // Disallow std::string overload
    bool should_include(const std::string &s) const = delete;

    virtual bool has_element(const eid_t /*id*/) const { return false; }

    virtual bool should_include(
        const namespace_ &ns, const std::string &name) const;

    /**
     * Return diagrams JSON context for inja templates.
     *
     * @return JSON context.
     */
    virtual inja::json context() const = 0;

    /**
     * @brief Check whether the diagram is empty
     *
     * @return True, if diagram is empty
     */
    virtual bool is_empty() const = 0;

private:
    std::string name_;
    std::unique_ptr<diagram_filter> filter_;
    bool complete_{false};
};

template <typename DiagramT> bool check_diagram_type(diagram_t t);
} // namespace clanguml::common::model
