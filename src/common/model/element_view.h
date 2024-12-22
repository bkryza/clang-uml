/**
 * @file src/common/model/element_view.h
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

#include "common/types.h"

#include <set>

namespace clanguml::common::model {

using clanguml::common::eid_t;

/**
 * Provides type based views over elements in a diagram.
 *
 * @tparam T Type of diagram element
 */
template <typename T> class element_view {
public:
    /**
     * @brief Add reference to diagram element
     *
     * @param element Reference to diagram element of specific type
     */
    void add(std::reference_wrapper<T> element)
    {
        elements_.emplace_back(std::move(element));
    }

    /**
     * @brief Get collection of reference to diagram elements
     *
     * @return
     */
    const reference_vector<T> &view() const { return elements_; }

    /**
     * @brief Get collection of reference to diagram elements
     *
     * @return
     */
    reference_vector<T> &view() { return elements_; }

    /**
     * @brief Get typed diagram element by id
     * @param id Global id of a diagram element
     *
     * @return
     */
    common::optional_ref<T> get(eid_t id) const
    {
        for (const auto &e : elements_) {
            if (e.get().id() == id) {
                return {e};
            }
        }

        return {};
    }

    /**
     * @brief Check whether the element view is empty
     *
     * @return True, if the view does not contain any elements
     */
    bool is_empty() const { return elements_.empty(); }

    void remove(const std::set<eid_t> &element_ids)
    {
        elements_.erase(std::remove_if(elements_.begin(), elements_.end(),
                            [&element_ids](auto &&e) {
                                return element_ids.count(e.get().id()) > 0;
                            }),
            elements_.end());
    }

    template <typename F> void for_each(F &&f) const
    {
        for (const auto &e : elements_) {
            f(e.get());
        }
    }

private:
    reference_vector<T> elements_;
};

template <typename... Ts> struct element_views : public element_view<Ts>... {
    template <typename F> void for_all_elements(F &&f)
    {
        (f(element_view<Ts>::view()), ...);
    }

    template <typename F> void for_all_elements(F &&f) const
    {
        (f(element_view<Ts>::view()), ...);
    }

    template <typename T> const element_view<T> &view() const
    {
        return dynamic_cast<const element_view<T> &>(*this);
    }

    /**
     * @brief Calls `f` function on `e` if it can be dynamically casted to
     *        any type in the element_views
     * @tparam T Element type
     * @tparam F Function to call on e
     * @param e Pointer to element
     * @param f Function to call with `e` dynamically casted to one of types
     */
    template <typename T, typename F> void dynamic_apply(T *e, F &&f) const
    {
        if (e == nullptr)
            return;

        (
            [&] {
                if (auto *ptr = dynamic_cast<Ts *>(e); ptr) {
                    f(ptr);
                }
            }(),
            ...);
    }
};
} // namespace clanguml::common::model