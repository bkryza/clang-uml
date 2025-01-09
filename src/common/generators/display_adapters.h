/**
 * @file src/common/generators/display_adapters.h
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

#include "common/model/element.h"
#include "util/util.h"

#include <string>

namespace clanguml::common::generators {

namespace detail {

template <typename U>
auto has_name_impl(int) -> decltype(std::declval<U>().name(), std::true_type{});

template <typename> std::false_type has_name_impl(...);

template <typename U> struct has_name : decltype(has_name_impl<U>(0)) { };

template <typename U>
auto has_type_impl(int) -> decltype(std::declval<U>().type(), std::true_type{});

template <typename> std::false_type has_type_impl(...);

template <typename U> struct has_type : decltype(has_type_impl<U>(0)) { };

template <typename U>
auto has_full_name_impl(
    int) -> decltype(std::declval<U>().full_name(std::declval<bool>()),
             std::true_type{});
template <typename> std::false_type has_full_name_impl(...);

template <typename U>
struct has_full_name : decltype(has_full_name_impl<U>(0)) { };

template <typename U>
auto has_name_no_ns_impl(
    int) -> decltype(std::declval<U>().name_no_ns(), std::true_type{});
template <typename> std::false_type has_name_no_ns_impl(...);

template <typename U>
struct has_name_no_ns : decltype(has_name_no_ns_impl<U>(0)) { };

template <typename U>
auto has_full_name_no_ns_impl(
    int) -> decltype(std::declval<U>().full_name_no_ns(), std::true_type{});
template <typename> std::false_type has_full_name_no_ns_impl(...);

template <typename U>
struct has_full_name_no_ns : decltype(has_full_name_no_ns_impl<U>(0)) { };

} // namespace detail

template <typename T> class display_name_adapter {
public:
    explicit display_name_adapter(const T &e)
        : element_{e}
    {
    }

    const display_name_adapter<T> &with_packages() const
    {
        with_packages_ = true;
        return *this;
    }

    template <typename U = T>
    std::enable_if_t<detail::has_name<U>::value, std::string> name() const
    {
        return adapt(element_.name());
    }

    template <typename U = T>
    std::enable_if_t<detail::has_type<U>::value, std::string> type() const
    {
        return adapt(element_.type());
    }

    template <typename U = T>
    std::enable_if_t<detail::has_full_name<U>::value, std::string> full_name(
        bool relative) const
    {
        return adapt(element_.full_name(relative));
    }

    template <typename U = T>
    std::enable_if_t<detail::has_name_no_ns<U>::value, std::string>
    name_no_ns() const
    {
        return adapt(element_.name_no_ns());
    }

    template <typename U = T>
    std::enable_if_t<detail::has_full_name_no_ns<U>::value, std::string>
    full_name_no_ns() const
    {
        return adapt(element_.full_name_no_ns());
    }

protected:
    std::string adapt(std::string n) const
    {
        util::replace_all(n, "##", "::");

        if constexpr (std::is_base_of<common::model::element, T>::value) {
            if (!with_packages_) {
                if (needs_root_prefix(element_))
                    return fmt::format("::{}", n);
            }
            else {
                if (element_.get_namespace().is_empty() &&
                    needs_root_prefix(element_))
                    return fmt::format("::{}", n);
            }
        }

        return n;
    }

private:
    mutable bool with_packages_{false};
    const T &element_;
};

} // namespace clanguml::common::generators