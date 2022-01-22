/**
 * src/package_diagram/model/class.h
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

#include "common/model/element.h"
#include "common/model/stylable_element.h"
#include "util/util.h"

#include <spdlog/spdlog.h>
#include <type_safe/optional_ref.hpp>

#include <string>
#include <vector>

namespace clanguml::package_diagram::model {

namespace detail {
template <typename T, template <typename> class Container,
    typename Ptr = std::unique_ptr<T>>
class package_trait {
public:
    void add_package(std::unique_ptr<T> p)
    {
        packages_.emplace_back(std::move(p));
    }

    void add_package(std::vector<std::string> path, std::unique_ptr<T> p)
    {
        assert(p);

        LOG_DBG(
            "Adding package {} at path '{}'", p->name(), fmt::join(path, "::"));

        if (path.empty()) {
            add_package(std::move(p));
            return;
        }

        auto parent = get_package(path);

        if (parent)
            parent.value().add_package(std::move(p));
        else
            spdlog::error(
                "No parent package found at: {}", fmt::join(path, "::"));
    }

    type_safe::optional_ref<T> get_package(std::vector<std::string> path) const
    {
        LOG_DBG("Getting package at path: {}", fmt::join(path, "::"));

        if (path.empty() || !has_package(path.at(0)))
            return {};

        auto p = get_package(path.at(0));
        if (path.size() == 1)
            return p;

        return p.value().get_package(
            std::vector<std::string>(path.begin() + 1, path.end()));
    }

    type_safe::optional_ref<T> get_package(const std::string &name) const
    {
        auto it = std::find_if(packages_.cbegin(), packages_.cend(),
            [&](const auto &p) { return name == p->name(); });

        if (it == packages_.end())
            return {};

        assert(it->get() != nullptr);

        return type_safe::ref(*(it->get()));
    }

    bool has_package(const std::string &name) const
    {
        return std::find_if(packages_.cbegin(), packages_.cend(),
                   [&](const auto &p) { return name == p->name(); }) !=
            packages_.end();
    }

    typedef typename Container<Ptr>::iterator iterator;
    typedef typename Container<Ptr>::const_iterator const_iterator;

    inline iterator begin() noexcept { return packages_.begin(); }
    inline const_iterator cbegin() const noexcept { return packages_.cbegin(); }
    inline iterator end() noexcept { return packages_.end(); }
    inline const_iterator cend() const noexcept { return packages_.cend(); }

protected:
    Container<std::unique_ptr<T>> packages_;
};
}

class package : public common::model::element,
                public common::model::stylable_element,
                public detail::package_trait<package, std::vector> {
public:
    package(const std::vector<std::string> &using_namespaces);

    std::string full_name(bool relative) const override;

    friend bool operator==(const package &l, const package &r);
};
}
