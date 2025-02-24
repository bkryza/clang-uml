/**
 * @file src/common/model/nested_trait.h
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

#include "util/util.h"

#include <iostream>
#include <list>
#include <optional>
#include <set>
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

    bool is_root() const { return is_root_; }

    void is_root(bool a) { is_root_ = a; }

    /**
     * Add element at the current nested level.
     *
     * @tparam V Type of element
     * @param p Element
     * @return True, if element was added.
     */
    template <typename V = T>
    [[nodiscard]] bool add_element(std::unique_ptr<V> element)
    {
        const auto element_id = element->id();
        const auto element_name = element->name();

        if (elements_by_id_.count(element_id) > 0) {
            // Element already in element tree
            return false;
        }

        elements_.push_back(std::move(element));

        auto element_it = std::prev(elements_.end());

        elements_by_id_.emplace(element_id, element_it);
        elements_by_name_.emplace(element_name, element_it);

        assert(elements_.size() == elements_by_id_.size());
        assert(elements_.size() == elements_by_name_.size());

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

        LOG_TRACE("Adding nested element {} at path '{}{}'", p->name(),
            path.is_root() ? "::" : "", path.to_string());

        if (path.is_empty()) {
            return add_element(std::move(p));
        }

        auto parent = get_element(path);

        if (parent) {
            auto *nested_trait_ptr =
                dynamic_cast<nested_trait<T, Path> *>(&parent.value());
            if (nested_trait_ptr != nullptr) {
                p->set_parent_element_id(parent.value().id());
                return (*nested_trait_ptr)
                    .template add_element<V>(std::move(p));
            }
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
            return get_element<V>(path[0], path.is_root());
        }

        auto p = get_element<T>(path[0], path.is_root());

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
    template <typename V = T>
    auto get_element(const std::string &name, bool is_root = false) const
    {
        assert(!util::contains(name, "::"));

        // Try to find an element by name and assuming it is a specific type
        // For some reason it is legal to have a C/C++ struct with the same
        // name as some ObjC protocol/interface, so the name is not
        // necessarily unique

        auto [it, matches_end] = elements_by_name_.equal_range(name);

        while (true) {
            if (it == matches_end)
                break;

            auto element_it = it->second;
            it++;

            if (auto element_ptr = dynamic_cast<V *>(element_it->get());
                element_ptr != nullptr) {
                if (auto nt_ptr = dynamic_cast<nested_trait<T, Path> *>(
                        element_it->get());
                    nt_ptr != nullptr) {
                    if (nt_ptr->is_root_ == is_root)
                        return optional_ref<V>{std::ref<V>(*element_ptr)};
                }
                else
                    return optional_ref<V>{std::ref<V>(*element_ptr)};
            }
        }

        return optional_ref<V>{};
    }

    /**
     * Return result of functor f applied to all_of elements.
     * @tparam F Functor type
     * @param f Functor value
     * @return True, if functor return true for elements, including nested
     * ones.
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
    bool is_empty(bool include_inner_packages = false) const
    {
        // If we're interested whether the tree contains any elements including
        // packages (even if their empty) just check elements_
        if (include_inner_packages)
            return elements_.empty();

        // If we're interested only in non-package elements, that we have to
        // traverse the nested chain
        return elements_.empty() ||
            std::all_of(elements_.cbegin(), elements_.cend(), [](auto &e) {
                const auto *package_ptr =
                    dynamic_cast<nested_trait<T, Path> *>(e.get());
                return package_ptr != nullptr && package_ptr->is_empty();
            });
    }

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
    void print_tree(const int level) const
    {
        if (level == 0) {
            std::cout << "--- Printing tree:\n";
        }

        for (auto it = elements_.cbegin(); it != elements_.cend(); it++) {
            const auto &e = *it;
            if (dynamic_cast<nested_trait<T, Path> *>(e.get())) {
                std::cout << std::string(level, ' ');
                std::cout << "[" << *e << "]\n";
                dynamic_cast<nested_trait<T, Path> *>(e.get())->print_tree(
                    level + 2);
            }
            else {
                std::cout << std::string(level, ' ');
                std::cout << "- " << *e << "]\n";
            }
        }
    }

    template <typename V = T> std::unique_ptr<V> get_and_remove(eid_t id)
    {
        std::unique_ptr<V> result;

        auto id_it = elements_by_id_.find(id);
        if (id_it == elements_by_id_.end()) {
            for (auto &p : elements_) {
                if (dynamic_cast<nested_trait<T, Path> *>(p.get())) {
                    result = dynamic_cast<nested_trait<T, Path> *>(p.get())
                                 ->get_and_remove<V>(id);
                    if (result)
                        break;
                }
            }
            return result;
        }

        // Get the iterator into the elements_ storage.
        auto element_it = id_it->second;

        // Remove from the name index.
        auto name = (*element_it)->name();
        auto name_range = elements_by_name_.equal_range(name);
        for (auto it = name_range.first; it != name_range.second; ++it) {
            if (it->second == element_it) {
                elements_by_name_.erase(it);
                break;
            }
        }

        elements_by_id_.erase(id);

        result = util::unique_pointer_cast<V>(std::move(*element_it));

        elements_.erase(element_it);

        assert(result);

        assert(elements_.size() == elements_by_id_.size());
        assert(elements_.size() == elements_by_name_.size());

        return result;
    }

    void remove(const std::set<eid_t> &element_ids)
    {
        for (const auto id : element_ids) {
            get_and_remove(id);
        }
    }

private:
    /**
     * Returns true of this nested level contains an element with specified
     * name.
     *
     * @param name Name of the element.
     * @return True if element exists.
     */
    bool has_element(const std::string &name) const
    {
        return elements_by_name_.count(name) != 0U;
    }

    bool is_root_{false};

    std::list<std::unique_ptr<T>> elements_;
    using element_iterator_t = typename std::list<std::unique_ptr<T>>::iterator;

    std::map<eid_t, element_iterator_t> elements_by_id_;
    std::multimap<std::string, element_iterator_t> elements_by_name_;
};

} // namespace clanguml::common::model
