/**
 * src/class_diagram/visitor/translation_unit_visitor.h
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

#include <cassert>
#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

namespace clanguml::common {

using id_t = int64_t;

template <typename T> class optional_ref {
public:
    using optional_type = T;

    optional_ref() = default;

    optional_ref(T *value) { value_ = value; }

    optional_ref(const T *value) { value_ = value; }

    optional_ref(T &value) { value_ = &value; }

    optional_ref(const T &value) { value_ = &value; }

    optional_ref(optional_ref &right) { value_ = right.get(); }

    template <typename V,
        typename = std::enable_if<
            std::is_base_of_v<optional_type, typename V::optional_type> ||
            std::is_same_v<optional_type, typename V::optional_type>>>
    optional_ref(const V &t)
    {
        value_ = t.get();
    }

    template <typename V,
        typename = std::enable_if<
            std::is_base_of_v<optional_type, typename V::optional_type> ||
            std::is_same_v<optional_type, typename V::optional_type>>>
    optional_ref(V &&t)
    {
        value_ = t.get();
        t.reset();
    }

    template <typename V,
        typename = std::enable_if<std::is_base_of_v<optional_type, V>>>
    optional_ref(const std::reference_wrapper<V> &t)
    {
        value_ = &t.get();
    }

    optional_ref &operator=(const optional_ref &right)
    {
        if (this == &right)
            return *this;

        value_ = right.value_;
        return *this;
    }

    optional_ref &operator=(optional_ref &&right) noexcept
    {
        if (this == &right)
            return *this;

        value_ = right.value_;
        right.reset();
        return *this;
    }

    bool has_value() const noexcept { return value_ != nullptr; }

    operator bool() const noexcept { return has_value(); }

    const T &value() const
    {
        assert(value_ != nullptr);
        return *value_;
    }

    T &value()
    {
        assert(value_ != nullptr);
        return *value_;
    }

    void reset() { value_ = nullptr; }

    T *get() const { return value_; }

private:
    T *value_{nullptr};
};

template <typename T>
using reference_vector = std::vector<std::reference_wrapper<T>>;

template <typename T>
using reference_set = std::unordered_set<std::reference_wrapper<T>>;

} // namespace clanguml::common