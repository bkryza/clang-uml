/**
 * src/class_diagram/model/diagram.cc
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

#include "diagram.h"

#include "util/error.h"
#include "util/util.h"

#include <cassert>

namespace clanguml::class_diagram::model {

const common::reference_vector<class_> &diagram::classes() const
{
    return classes_;
}

const common::reference_vector<enum_> &diagram::enums() const { return enums_; }

const common::reference_vector<concept_> &diagram::concepts() const
{
    return concepts_;
}

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kClass;
}

common::optional_ref<clanguml::common::model::diagram_element> diagram::get(
    const std::string &full_name) const
{
    common::optional_ref<clanguml::common::model::diagram_element> res =
        get_class(full_name);

    if (res.has_value())
        return res;

    res = get_enum(full_name);

    if (res.has_value())
        return res;

    res = get_concept(full_name);

    return res;
}

common::optional_ref<clanguml::common::model::diagram_element> diagram::get(
    const clanguml::common::model::diagram_element::id_t id) const
{
    common::optional_ref<clanguml::common::model::diagram_element> res;

    res = get_class(id);

    if (res.has_value())
        return res;

    res = get_enum(id);

    if (res.has_value())
        return res;

    res = get_concept(id);

    return res;
}

bool diagram::has_class(const class_ &c) const
{
    return std::any_of(classes_.cbegin(), classes_.cend(),
        [&c](const auto &cc) { return cc.get() == c; });
}

bool diagram::has_enum(const enum_ &e) const
{
    return std::any_of(enums_.cbegin(), enums_.cend(),
        [&e](const auto &ee) { return ee.get().full_name() == e.full_name(); });
}

bool diagram::has_concept(const concept_ &c) const
{
    return std::any_of(concepts_.cbegin(), concepts_.cend(),
        [&c](const auto &cc) { return cc.get() == c; });
}

common::optional_ref<class_> diagram::get_class(const std::string &name) const
{
    for (const auto &c : classes_) {
        const auto full_name = c.get().full_name(false);

        if (full_name == name) {
            return {c};
        }
    }

    return {};
}

common::optional_ref<class_> diagram::get_class(
    clanguml::common::model::diagram_element::id_t id) const
{
    for (const auto &c : classes_) {
        if (c.get().id() == id) {
            return {c};
        }
    }

    return {};
}

common::optional_ref<enum_> diagram::get_enum(const std::string &name) const
{
    for (const auto &e : enums_) {
        if (e.get().full_name(false) == name) {
            return {e};
        }
    }

    return {};
}

common::optional_ref<enum_> diagram::get_enum(
    clanguml::common::model::diagram_element::id_t id) const
{
    for (const auto &e : enums_) {
        if (e.get().id() == id) {
            return {e};
        }
    }

    return {};
}

common::optional_ref<concept_> diagram::get_concept(
    const std::string &name) const
{
    for (const auto &c : concepts_) {
        const auto full_name = c.get().full_name(false);

        if (full_name == name) {
            return {c};
        }
    }

    return {};
}

common::optional_ref<concept_> diagram::get_concept(
    clanguml::common::model::diagram_element::id_t id) const
{
    for (const auto &c : concepts_) {
        if (c.get().id() == id) {
            return {c};
        }
    }

    return {};
}

bool diagram::add_class(
    const common::model::path &parent_path, std::unique_ptr<class_> &&c)
{
    if (parent_path.type() == common::model::path_type::kNamespace) {
        return add_class_ns(std::move(c));
    }

    return add_class_fs(parent_path, std::move(c));
}

bool diagram::add_enum(const path &parent_path, std::unique_ptr<enum_> &&e)
{
    if (parent_path.type() == common::model::path_type::kNamespace) {
        return add_enum_ns(std::move(e));
    }

    return add_enum_fs(parent_path, std::move(e));
}

bool diagram::add_concept(
    const path &parent_path, std::unique_ptr<concept_> &&c)
{
    if (parent_path.type() == common::model::path_type::kNamespace) {
        return add_concept_ns(std::move(c));
    }

    return add_concept_fs(parent_path, std::move(c));
}

bool diagram::add_package(
    const path &parent_path, std::unique_ptr<common::model::package> &&p)
{
    if (parent_path.type() == common::model::path_type::kNamespace) {
        return add_package_ns(std::move(p));
    }

    return add_package_fs(parent_path, std::move(p));
}

bool diagram::add_package_fs(
    const path &parent_path, std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding filesystem package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();

    return add_element(ns, std::move(p));
}

bool diagram::add_package_ns(std::unique_ptr<common::model::package> &&p)
{
    LOG_DBG("Adding namespace package: {}, {}", p->name(), p->full_name(true));

    auto ns = p->get_relative_namespace();

    return add_element(ns, std::move(p));
}

bool diagram::add_class_fs(
    const common::model::path &parent_path, std::unique_ptr<class_> &&c)
{
    // Make sure all parent directories are already packages in the
    // model
    for (auto it = parent_path.begin(); it != parent_path.end(); it++) {
        auto pkg =
            std::make_unique<common::model::package>(c->using_namespace());
        pkg->set_name(*it);
        auto ns = common::model::path(parent_path.begin(), it);
        // ns.pop_back();
        pkg->set_namespace(ns);
        pkg->set_id(common::to_id(pkg->full_name(false)));

        add_package_fs(ns, std::move(pkg));
    }

    const auto base_name = c->name();
    const auto full_name = c->full_name(false);
    const auto id = c->id();
    auto cc = std::ref(*c);

    if (add_element(parent_path, std::move(c))) {
        classes_.push_back(cc);
        return true;
    }
    else {
        LOG_WARN("Cannot add class {} with id {} due to: {}", base_name, id);
    }

    return false;
}

bool diagram::add_class_ns(std::unique_ptr<class_> &&c)
{
    const auto base_name = c->name();
    const auto full_name = c->full_name(false);

    LOG_DBG("Adding class: {}::{}, {}", c->get_namespace().to_string(),
        base_name, full_name);

    if (util::contains(base_name, "::"))
        throw std::runtime_error("Name cannot contain namespace: " + base_name);

    if (util::contains(base_name, "*"))
        throw std::runtime_error("Name cannot contain *: " + base_name);

    const auto ns = c->get_relative_namespace();
    auto name = base_name;
    auto name_with_ns = c->name_and_ns();
    auto name_and_ns = ns | name;
    auto &cc = *c;
    auto id = cc.id();

    try {
        if (!has_class(cc)) {
            if (add_element(ns, std::move(c)))
                classes_.push_back(std::ref(cc));

            const auto &el = get_element<class_>(name_and_ns).value();

            if ((el.name() != name) || !(el.get_relative_namespace() == ns))
                throw std::runtime_error(
                    "Invalid element stored in the diagram tree");

            LOG_DBG("Added class {} ({} - [{}])", base_name, full_name, id);

            return true;
        }
    }
    catch (const std::runtime_error &e) {
        LOG_WARN(
            "Cannot add class {} with id {} due to: {}", name, id, e.what());
        return false;
    }

    LOG_DBG(
        "Class {} ({} - [{}]) already in the model", base_name, full_name, id);

    return false;
}

bool diagram::add_enum_ns(std::unique_ptr<enum_> &&e)
{
    const auto full_name = e->name();

    LOG_DBG("Adding enum: {}", full_name);

    assert(!util::contains(e->name(), "::"));

    auto e_ref = std::ref(*e);
    auto ns = e->get_relative_namespace();

    if (!has_enum(*e)) {
        if (add_element(ns, std::move(e))) {
            enums_.emplace_back(e_ref);
            return true;
        }
    }

    LOG_DBG("Enum {} already in the model", full_name);

    return false;
}

bool diagram::add_enum_fs(
    const common::model::path &parent_path, std::unique_ptr<enum_> &&e)
{
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

        add_package_fs(ns, std::move(pkg));
    }

    const auto base_name = e->name();
    const auto full_name = e->full_name(false);
    const auto id = e->id();
    auto &cc = *e;

    if (add_element(parent_path, std::move(e))) {
        enums_.push_back(std::ref(cc));
        return true;
    }
    else {
        LOG_WARN("Cannot add class {} with id {} due to: {}", base_name, id);
    }

    return false;
}

bool diagram::add_concept_ns(std::unique_ptr<concept_> &&c)
{
    const auto base_name = c->name();
    const auto full_name = c->full_name(false);

    LOG_DBG("Adding concept: {}::{}, {}", c->get_namespace().to_string(),
        base_name, full_name);

    if (util::contains(base_name, "::"))
        throw std::runtime_error("Name cannot contain namespace: " + base_name);

    if (util::contains(base_name, "*"))
        throw std::runtime_error("Name cannot contain *: " + base_name);

    const auto ns = c->get_relative_namespace();
    auto name = base_name;
    auto name_with_ns = c->name_and_ns();
    auto name_and_ns = ns | name;
    auto &cc = *c;
    auto id = cc.id();

    try {
        if (!has_concept(cc)) {
            if (add_element(ns, std::move(c)))
                concepts_.push_back(std::ref(cc));

            const auto &el = get_element<concept_>(name_and_ns).value();

            if ((el.name() != name) || !(el.get_relative_namespace() == ns))
                throw std::runtime_error(
                    "Invalid element stored in the diagram tree");

            LOG_DBG("Added concept {} ({} - [{}])", base_name, full_name, id);

            return true;
        }
    }
    catch (const std::runtime_error &e) {
        LOG_WARN(
            "Cannot add concept {} with id {} due to: {}", name, id, e.what());
        return false;
    }

    LOG_DBG("Concept {} ({} - [{}]) already in the model", base_name, full_name,
        id);

    return false;
}

bool diagram::add_concept_fs(
    const common::model::path &parent_path, std::unique_ptr<concept_> &&c)
{
    // Make sure all parent directories are already packages in the
    // model
    for (auto it = parent_path.begin(); it != parent_path.end(); it++) {
        auto pkg =
            std::make_unique<common::model::package>(c->using_namespace());
        pkg->set_name(*it);
        auto ns = common::model::path(parent_path.begin(), it);
        // ns.pop_back();
        pkg->set_namespace(ns);
        pkg->set_id(common::to_id(pkg->full_name(false)));

        add_package_fs(ns, std::move(pkg));
    }

    const auto base_name = c->name();
    const auto full_name = c->full_name(false);
    const auto id = c->id();
    auto &cc = *c;

    if (add_element(parent_path, std::move(c))) {
        concepts_.push_back(std::ref(cc));
        return true;
    }

    LOG_WARN("Cannot add class {} with id {} due to: {}", base_name, id);

    return false;
}

void diagram::get_parents(
    clanguml::common::reference_set<class_> &parents) const
{
    bool found_new{false};
    for (const auto &parent : parents) {
        for (const auto &pp : parent.get().parents()) {
            auto p = get_class(pp.id());

            if (p.has_value()) {
                auto [it, found] = parents.emplace(std::ref(p.value()));
                if (found)
                    found_new = true;
            }
        }
    }

    if (found_new) {
        get_parents(parents);
    }
}

bool diagram::has_element(
    clanguml::common::model::diagram_element::id_t id) const
{
    const auto has_class = std::any_of(classes_.begin(), classes_.end(),
        [id](const auto &c) { return c.get().id() == id; });

    if (has_class)
        return true;

    return std::any_of(enums_.begin(), enums_.end(),
        [id](const auto &c) { return c.get().id() == id; });
}

std::string diagram::to_alias(
    clanguml::common::model::diagram_element::id_t id) const
{
    LOG_DBG("Looking for alias for {}", id);

    for (const auto &c : classes_) {
        if (c.get().id() == id) {
            return c.get().alias();
        }
    }

    for (const auto &e : enums_) {
        if (e.get().id() == id)
            return e.get().alias();
    }

    for (const auto &c : concepts_) {
        if (c.get().id() == id)
            return c.get().alias();
    }

    throw error::uml_alias_missing(fmt::format("Missing alias for {}", id));
}

inja::json diagram::context() const
{
    inja::json ctx;
    ctx["name"] = name();
    ctx["type"] = "class";

    inja::json::array_t elements{};

    // Add classes
    for (const auto &c : classes()) {
        elements.emplace_back(c.get().context());
    }

    // Add enums
    for (const auto &e : enums()) {
        elements.emplace_back(e.get().context());
    }

    ctx["elements"] = elements;

    return ctx;
}

} // namespace clanguml::class_diagram::model

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::class_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kClass;
}
}
