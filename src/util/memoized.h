/**
 * src/util/memoized.h
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

#include <map>
#include <optional>
#include <tuple>

namespace clanguml::util {
/**
 * @brief Simple memoization implementation for expensive methods.
 *
 * @tparam T Tag type to allow multiple memoizations per class
 * @tparam Ret Return type of the memoized method F
 * @tparam Args Arguments the memoized method F
 */
template <typename T, typename Ret, typename... Args> class memoized {
public:
    using key_t = std::tuple<Args...>;
    using value_t = Ret;

    template <typename F>
    auto memoize(bool is_complete, F &&f, Args... args) const
    {
        if (!is_complete)
            return f(std::forward<Args>(args)...);

        const auto key = key_t{std::forward<Args>(args)...};
        if (cache_.find(key) == cache_.end())
            cache_[key] = std::apply(f, key);

        return cache_.at(key);
    }

    void invalidate(Args... args) const { cache_.erase(args...); }

private:
    mutable std::map<key_t, value_t> cache_;
};

template <typename T, typename Ret> class memoized<T, Ret> {
public:
    using key_t = bool;
    using value_t = Ret;

    template <typename F> auto memoize(bool is_complete, F f) const
    {
        if (!is_complete)
            return f();

        if (!value_) {
            value_ = f();
        }

        return *value_; // NOLINT
    }

    void invalidate() const { value_.reset(); }

private:
    mutable std::optional<Ret> value_;
};

template <typename T, typename Ret> class memoized<T, Ret, bool> {
public:
    using key_t = bool;
    using value_t = Ret;

    template <typename F> auto memoize(bool is_complete, F f, bool arg) const
    {
        if (!is_complete)
            return f(arg);

        const auto key = arg;
        if (key && !true_value_) {
            true_value_ = f(arg);
        }

        if (!key && !false_value_) {
            false_value_ = f(arg);
        }

        return key ? *true_value_ : *false_value_; // NOLINT
    }

    void invalidate(bool key) const
    {
        if (key)
            true_value_.reset();
        else
            false_value_.reset();
    }

private:
    mutable std::optional<Ret> true_value_;
    mutable std::optional<Ret> false_value_;
};
} // namespace clanguml::util