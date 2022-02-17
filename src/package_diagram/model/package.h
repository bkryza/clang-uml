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

#include <set>
#include <string>
#include <vector>

namespace clanguml::package_diagram::model {

namespace detail {
template <typename T> class package_trait {
public:
    package_trait() = default;

    package_trait(const package_trait &) = delete;
    package_trait(package_trait &&) = default;

    package_trait &operator=(const package_trait &) = delete;
    package_trait &operator=(package_trait &&) = default;

    virtual ~package_trait() = default;

    void add_package(std::unique_ptr<T> p)
    {
        auto it = std::find_if(packages_.begin(), packages_.end(),
            [&p](const auto &e) { return *e == *p; });

        if (it != packages_.end()) {
            (*it)->append(*p);
        }
        else {
            packages_.emplace_back(std::move(p));
        }
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

        if (path.empty() || !has_package(path.at(0))) {
            LOG_WARN(
                "Sub package {} not found in package", fmt::join(path, "::"));
            return {};
        }

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

    auto begin() { return packages_.begin(); }
    auto end() { return packages_.end(); }

    auto cbegin() const { return packages_.cbegin(); }
    auto cend() const { return packages_.cend(); }

    auto begin() const { return packages_.begin(); }
    auto end() const { return packages_.end(); }

private:
    std::vector<std::unique_ptr<T>> packages_;
};
}

class package : public common::model::element,
                public common::model::stylable_element,
                public detail::package_trait<package> {
public:
    package(const std::vector<std::string> &using_namespaces);

    package(const package &) = delete;
    package(package &&) = default;

    package &operator=(const package &) = delete;

    std::string full_name(bool relative) const override;

    friend bool operator==(const package &l, const package &r);

    bool is_deprecated() const;

    void set_deprecated(bool deprecated);

private:
    bool is_deprecated_{false};
};
}
