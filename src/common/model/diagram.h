/**
 * @file src/common/model/diagram.h
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

#include "common/model/element_view.h"
#include "diagram_element.h"
#include "enums.h"
#include "namespace.h"
#include "source_file.h"
#include "util/error.h"

#include <algorithm>
#include <iterator>
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
    diagram(const config::diagram &config);

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

    bool should_include(
        const namespace_ &ns, const std::string &name) const;

    /**
     * @brief Check whether the diagram is empty
     *
     * @return True, if diagram is empty
     */
    virtual bool is_empty() const = 0;

    virtual void apply_filter() { }

    /**
     * After generating the diagram we need to make sure that there are no
     * duplicate relationships in the diagram.
     */
    void remove_duplicate_relationships();

    /**
     * Get diagram filter
     *
     * @return Reference to the diagrams element filter
     */
    diagram_filter &filter() { return *filter_; }

private:
    /**
     * Set diagram filter for this diagram.
     *
     * @param filter diagram_filter instance
     *
     * @see clanguml::common::model::diagram_filter
     */
    void set_filter(std::unique_ptr<diagram_filter> filter);

protected:
    void append(diagram &&other);

private:
    std::string name_;
    std::unique_ptr<diagram_filter> filter_;

    bool complete_{false};
    bool filtered_{false};
};

template <typename... Ts>
class hierarchical_diagram
    : public diagram,
      public clanguml::common::model::element_views<Ts...> {
public:
    hierarchical_diagram(const config::diagram &config)
        : common::model::diagram(config)
    {
    }

    template <typename ElementT> void remove(eid_t id)
    {
        element_view<ElementT>::remove({id});
        elements_.erase(id);
    }

    /**
     * @brief Convert element id to PlantUML alias.
     *
     * @todo This method does not belong here - refactor to PlantUML specific
     *       code.
     *
     * @param id Id of the diagram element.
     * @return PlantUML alias.
     */
    std::string to_alias(eid_t id) const
    {
        LOG_TRACE("Looking for alias for {}", id);

        if (elements_.count(id) == 0)
            throw error::uml_alias_missing(
                fmt::format("Missing alias for {}", id));

        return elements_.at(id)->alias();
    }

    /**
     * Return all relationships outgoing from this element.
     *
     * @return List of relationships.
     */
    std::vector<relationship> &relationships() { return relationships_; }

    /**
     * Return all relationships outgoing from this element.
     *
     * @return List of relationships.
     */
    const std::vector<relationship> &relationships() const
    {
        return relationships_;
    }

    void remove_duplicate_relationships()
    {
        std::vector<relationship> unique_relationships;

        for (auto &r : relationships_) {
            if (!util::contains(unique_relationships, r)) {
                unique_relationships.emplace_back(r);
            }
        }

        std::swap(relationships_, unique_relationships);
    }

    /**
     * Add relationships, whose source is this element.
     *
     * @param cr Relationship to another diagram element.
     */
    void add_relationship(relationship &&cr)
    {
        assert(cr.source().has_value());

        if ((cr.type() == relationship_t::kInstantiation) &&
            (cr.destination() == cr.source())) {
            LOG_DBG("Skipping self instantiation relationship for {}",
                cr.destination());
            return;
        }

        if (!util::contains(relationships_, cr)) {
            LOG_DBG("Adding relationship from: '{}' - {} - '{}'", cr.source(),
                to_string(cr.type()), cr.destination());

            relationships_.emplace_back(std::move(cr));
        }
    }

    /**
     * Return an iterator range over relationships with matching source id.
     *
     * @param source_id The source element id to filter by.
     * @return Iterator range over relationships with matching source.
     */
    auto relationships(eid_t source_id) const
    {
        class filtered_iterator {
        private:
            std::vector<relationship>::const_iterator current_;
            std::vector<relationship>::const_iterator end_;
            eid_t source_id_;

            void advance_to_next_match()
            {
                while (current_ != end_ && current_->source() != source_id_) {
                    ++current_;
                }
            }

        public:
            filtered_iterator(std::vector<relationship>::const_iterator start,
                std::vector<relationship>::const_iterator end, eid_t source_id)
                : current_(start)
                , end_(end)
                , source_id_(std::move(source_id))
            {
                advance_to_next_match();
            }

            filtered_iterator &operator++()
            {
                if (current_ != end_) {
                    ++current_;
                    advance_to_next_match();
                }
                return *this;
            }

            const relationship &operator*() const { return *current_; }

            const relationship *operator->() const { return &(*current_); }

            bool operator!=(const filtered_iterator &other) const
            {
                return current_ != other.current_;
            }

            bool operator==(const filtered_iterator &other) const
            {
                return current_ == other.current_;
            }
        };

        struct filtered_range {
            filtered_iterator begin_iter;
            filtered_iterator end_iter;

            filtered_iterator begin() const { return begin_iter; }
            filtered_iterator end() const { return end_iter; }
        };

        return filtered_range{filtered_iterator(relationships_.begin(),
                                  relationships_.end(), source_id),
            filtered_iterator(
                relationships_.end(), relationships_.end(), source_id)};
    }

    void apply_filter() override
    {
        //        clanguml::common::model::apply_filter(relationships_,
        //        filter());

        // First find all element ids which should be removed
//        std::set<eid_t> to_remove;
//
//        while (true) {
//            for_all_elements([&](auto &&elements_view) mutable {
//                for (const auto &el : elements_view)
//                    if (!filter().should_include(el.get()))
//                        to_remove.emplace(el.get().id());
//            });
//
//            if (to_remove.empty())
//                break;
//
//            for_each_view([&to_remove](auto &view) { view.remove(to_remove); });
//
//            for (const auto id : to_remove) {
//                elements_.erase(id);
//            }
//
//            to_remove.clear();
//
//            filter().reset();
//        }
//
//        for_all_elements([&](auto &&elements_view) mutable {
//            for (const auto &el : elements_view)
//                el.get().apply_filter(filter(), to_remove);
//        });
//
//        auto &rels = relationships();
//        rels.erase(std::remove_if(std::begin(rels), std::end(rels),
//                       [&to_remove](auto &&r) {
//                           return to_remove.count(r.source()) > 0 ||
//                               to_remove.count(r.destination()) > 0;
//                       }),
//            std::end(rels));
    }

protected:
    void append(hierarchical_diagram &&other)
    {
        assert(complete() && other.complete());
        assert(name() == other.name());

        for (auto &&r : std::move(other).relationships()) {
            add_relationship(std::move(r));
        }
    }

private:
    std::vector<relationship> relationships_;
    std::map<eid_t, std::unique_ptr<common::model::element>> elements_;
};

template <typename T> bool needs_root_prefix(const T &e)
{
    if (e.get_namespace().type() != model::path_type::kNamespace)
        return false;

    if (e.using_namespace().is_empty())
        return false;

    return !util::starts_with(
        e.full_name(false), e.using_namespace().to_string());
}

template <typename DiagramT> bool check_diagram_type(diagram_t t);
} // namespace clanguml::common::model
