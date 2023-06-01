/**
 * src/class_diagram/model/diagram.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
#include "common/model/diagram.h"
#include "common/model/element_view.h"
#include "common/model/nested_trait.h"
#include "common/model/package.h"
#include "common/types.h"
#include "concept.h"
#include "enum.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace clanguml::class_diagram::model {

using common::opt_ref;
using common::model::diagram_element;
using common::model::diagram_t;
using common::model::element_view;
using common::model::path;
using common::model::path_type;

using nested_trait_ns =
    clanguml::common::model::nested_trait<clanguml::common::model::element,
        clanguml::common::model::namespace_>;

class diagram : public common::model::diagram,
                public element_view<class_>,
                public element_view<enum_>,
                public element_view<concept_>,
                public nested_trait_ns {
public:
    diagram() = default;

    diagram(const diagram &) = delete;
    diagram(diagram &&) = default;
    diagram &operator=(const diagram &) = delete;
    diagram &operator=(diagram &&) = default;

    diagram_t type() const override;

    using common::model::diagram::should_include;

    bool should_include(const class_member &m) const;

    bool should_include(const class_method &m) const;

    opt_ref<diagram_element> get(const std::string &full_name) const override;

    opt_ref<diagram_element> get(diagram_element::id_t id) const override;

    const common::reference_vector<class_> &classes() const;

    const common::reference_vector<enum_> &enums() const;

    const common::reference_vector<concept_> &concepts() const;

    template <typename ElementT> bool contains(const ElementT &e);

    template <typename ElementT>
    opt_ref<ElementT> find(const std::string &name) const;

    template <typename ElementT>
    opt_ref<ElementT> find(diagram_element::id_t id) const;

    template <typename ElementT>
    bool add(const path &parent_path, std::unique_ptr<ElementT> &&e)
    {
        if (parent_path.type() == common::model::path_type::kNamespace) {
            return add_with_namespace_path(std::move(e));
        }

        return add_with_filesystem_path(parent_path, std::move(e));
    }

    std::string to_alias(diagram_element::id_t id) const;

    void get_parents(clanguml::common::reference_set<class_> &parents) const;

    friend void print_diagram_tree(const diagram &d, int level);

    bool has_element(diagram_element::id_t id) const override;

    inja::json context() const override;

private:
    template <typename ElementT>
    bool add_with_namespace_path(std::unique_ptr<ElementT> &&e);

    template <typename ElementT>
    bool add_with_filesystem_path(
        const common::model::path &parent_path, std::unique_ptr<ElementT> &&e);
};

template <typename ElementT> bool diagram::contains(const ElementT &element)
{
    return std::any_of(element_view<ElementT>::view().cbegin(),
        element_view<ElementT>::view().cend(),
        [&element](
            const auto &element_opt) { return element_opt.get() == element; });
}

template <typename ElementT>
bool diagram::add_with_namespace_path(std::unique_ptr<ElementT> &&e)
{
    const auto base_name = e->name();
    const auto full_name = e->full_name(false);
    const auto element_type = e->type_name();

    LOG_DBG("Adding {}: {}::{}, {}", element_type,
        e->get_namespace().to_string(), base_name, full_name);

    if (util::contains(base_name, "::"))
        throw std::runtime_error("Name cannot contain namespace: " + base_name);

    if (util::contains(base_name, "*"))
        throw std::runtime_error("Name cannot contain *: " + base_name);

    const auto ns = e->get_relative_namespace();
    auto name = base_name;
    auto name_with_ns = e->name_and_ns();
    auto name_and_ns = ns | name;
    auto &e_ref = *e;
    auto id = e_ref.id();

    try {
        if (!contains(e_ref)) {
            if (add_element(ns, std::move(e)))
                element_view<ElementT>::add(std::ref(e_ref));

            const auto &el = get_element<ElementT>(name_and_ns).value();

            if ((el.name() != name) || !(el.get_relative_namespace() == ns))
                throw std::runtime_error(
                    "Invalid element stored in the diagram tree");

            LOG_DBG("Added {} {} ({} - [{}])", element_type, base_name,
                full_name, id);

            return true;
        }
    }
    catch (const std::runtime_error &e) {
        LOG_WARN("Cannot add {} {} with id {} due to: {}", element_type, name,
            id, e.what());
        return false;
    }

    LOG_DBG("{} {} ({} - [{}]) already in the model", element_type, base_name,
        full_name, id);

    return false;
}

template <typename ElementT>
bool diagram::add_with_filesystem_path(
    const common::model::path &parent_path, std::unique_ptr<ElementT> &&e)
{
    const auto element_type = e->type_name();

    // Make sure all parent directories are already packages in the
    // model
    for (auto it = parent_path.begin(); it != parent_path.end(); it++) {
        auto pkg =
            std::make_unique<common::model::package>(e->using_namespace());
        pkg->set_name(*it);
        auto ns = common::model::path(parent_path.begin(), it);
        // ns.pop_back();
        pkg->set_namespace(ns);
        pkg->set_id(common::to_id(pkg->full_name(false)));

        add(ns, std::move(pkg));
    }

    const auto base_name = e->name();
    const auto full_name = e->full_name(false);
    auto &e_ref = *e;

    if (add_element(parent_path, std::move(e))) {
        element_view<ElementT>::add(std::ref(e_ref));
        return true;
    }

    return false;
}

template <typename ElementT>
opt_ref<ElementT> diagram::find(const std::string &name) const
{
    for (const auto &element : element_view<ElementT>::view()) {
        const auto full_name = element.get().full_name(false);

        if (full_name == name) {
            return {element};
        }
    }

    return {};
}

template <typename ElementT>
opt_ref<ElementT> diagram::find(diagram_element::id_t id) const
{
    for (const auto &element : element_view<ElementT>::view()) {
        if (element.get().id() == id) {
            return {element};
        }
    }

    return {};
}

//
// Template method specialization pre-declarations...
//
template <>
bool diagram::add_with_namespace_path<common::model::package>(
    std::unique_ptr<common::model::package> &&p);

template <>
bool diagram::add_with_filesystem_path<common::model::package>(
    const common::model::path &parent_path,
    std::unique_ptr<common::model::package> &&p);

} // namespace clanguml::class_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::class_diagram::model::diagram>(diagram_t t);
}