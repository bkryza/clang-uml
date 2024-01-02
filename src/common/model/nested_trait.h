/**
 * @file src/common/model/nested_trait.h
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

#include "util/util.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace clanguml::common::model {

/**
 * @brief Base class for elements nested in the diagram.
 *
 * This class provides a common trait for diagram elements which can contain
 * other nested elements, e.g. packages.
 *
 * @embed{nested_trait_hierarchy_class.svg}
 *
 * @tparam T Type of element
 * @tparam Path Type of nested path (e.g. namespace or directory path)
 */
template <typename T, typename Path> class nested_trait {
public:
    nested_trait() = default;

    nested_trait(const nested_trait &) = delete;
    nested_trait(nested_trait &&) noexcept = default;

    nested_trait &operator=(const nested_trait &) = delete;
    nested_trait &operator=(nested_trait &&) noexcept = default;

    virtual ~nested_trait() = default;

    /**
     * Add element at the current nested level.
     *
     * @tparam V Type of element
     * @param p Element
     * @return True, if element was added.
     */
    template <typename V = T>
    [[nodiscard]] bool add_element(std::unique_ptr<V> p)
    {
        auto it = std::find_if(elements_.begin(), elements_.end(),
            [&p](const auto &e) { return *e == *p; });

        if (it != elements_.end()) {
            // Element already in element tree
            return false;
        }

        elements_.emplace_back(std::move(p));

        return true;
    }

    /**
     * Add element at a nested path.
     *
     * @tparam V Type of element
     * @param path Nested path (e.g. list of namespaces)
     * @param p Element
     * @return True, if element was added.
     */
    template <typename V = T>
    bool add_element(const Path &path, std::unique_ptr<V> p)
    {
        assert(p);

        LOG_DBG("Adding nested element {} at path '{}'", p->name(),
            path.to_string());

        if (path.is_empty()) {
            return add_element(std::move(p));
        }

        auto parent = get_element(path);

        if (parent && dynamic_cast<nested_trait<T, Path> *>(&parent.value())) {
            p->set_parent_element_id(parent.value().id());
            return dynamic_cast<nested_trait<T, Path> &>(parent.value())
                .template add_element<V>(std::move(p));
        }

        LOG_INFO("No parent element found at: {}", path.to_string());

        throw std::runtime_error(
            "No parent element found for " + path.to_string());
    }

    /**
     * Get element at path, if exists.
     *
     * @tparam V Element type.
     * @param path Path to the element.
     * @return Optional reference to the element.
     */
    template <typename V = T> auto get_element(const Path &path) const
    {
        if (path.is_empty() || !has_element(path[0])) {
            LOG_DBG("Nested element {} not found in element", path.to_string());
            return optional_ref<V>{};
        }

        if (path.size() == 1) {
            return get_element<V>(path[0]);
        }

        auto p = get_element<T>(path[0]);

        if (!p)
            return optional_ref<V>{};

        if (dynamic_cast<nested_trait<T, Path> *>(&p.value()))
            return dynamic_cast<nested_trait<T, Path> &>(p.value())
                .get_element<V>(Path{path.begin() + 1, path.end()});

        return optional_ref<V>{};
    }

    /**
     * Get element by name at the current nested level.
     *
     * @tparam V Type of element.
     * @param name Name of the element (cannot contain namespace or path)
     * @return Optional reference to the element.
     */
    template <typename V = T> auto get_element(const std::string &name) const
    {
        assert(!util::contains(name, "::"));

        auto it = std::find_if(elements_.cbegin(), elements_.cend(),
            [&](const auto &p) { return name == p->name(); });

        if (it == elements_.end())
            return optional_ref<V>{};

        assert(it->get() != nullptr);

        if (dynamic_cast<V *>(it->get()))
            return optional_ref<V>{std::ref<V>(dynamic_cast<V &>(*it->get()))};

        return optional_ref<V>{};
    }

    /**
     * Returns true of this nested level contains an element with specified
     * name.
     *
     * @param name Name of the element.
     * @return True if element exists.
     */
    bool has_element(const std::string &name) const
    {
        return std::find_if(elements_.cbegin(), elements_.cend(),
                   [&](const auto &p) { return name == p->name(); }) !=
            elements_.end();
    }

    /**
     * Return result of functor f applied to all_of elements.
     * @tparam F Functor type
     * @param f Functor value
     * @return True, if functor return true for elements, including nested ones.
     */
    template <typename F> bool all_of(F &&f) const
    {
        return std::all_of(
            elements_.cbegin(), elements_.cend(), [f](const auto &e) {
                const auto *package_ptr =
                    dynamic_cast<nested_trait<T, Path> *>(e.get());

                if (package_ptr != nullptr)
                    return package_ptr->all_of(f);

                return f(*e);
            });
    }

    /**
     * Check if nested element is empty.
     *
     * @return True if this nested element is empty.
     */
    bool is_empty() const
    {
        return elements_.empty() ||
            std::all_of(elements_.cbegin(), elements_.cend(), [](auto &e) {
                const auto *package_ptr =
                    dynamic_cast<nested_trait<T, Path> *>(e.get());
                return package_ptr != nullptr && package_ptr->is_empty();
            });
    }

    auto begin() { return elements_.begin(); }
    auto end() { return elements_.end(); }

    auto cbegin() const { return elements_.cbegin(); }
    auto cend() const { return elements_.cend(); }

    auto begin() const { return elements_.begin(); }
    auto end() const { return elements_.end(); }

    /**
     * Print the nested trait in the form of a tree.
     *
     * This method is used for debugging only.
     *
     * @param level Tree level
     */
    void print_tree(const int level)
    {
        const auto &d = *this;

        if (level == 0) {
            std::cout << "--- Printing tree:\n";
        }
        for (const auto &e : d) {
            if (dynamic_cast<nested_trait<T, Path> *>(e.get())) {
                std::cout << std::string(level, ' ') << "[" << *e << "]\n";
                dynamic_cast<nested_trait<T, Path> *>(e.get())->print_tree(
                    level + 1);
            }
            else {
                std::cout << std::string(level, ' ') << "- " << *e << "]\n";
            }
        }
    }

private:
    std::vector<std::unique_ptr<T>> elements_;
};

} // namespace clanguml::common::model
