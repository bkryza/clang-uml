/**
 * src/common/model/path.h
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

#include "util/util.h"

#include <optional>
#include <string>
#include <vector>

namespace clanguml::common::model {

template <typename Sep> class path {
public:
    using container_type = std::vector<std::string>;

    path() = default;

    explicit path(const std::string &ns)
    {
        if (ns.empty())
            return;

        path_ = util::split(ns, Sep::value);
    }

    path(container_type::const_iterator begin,
        container_type::const_iterator end)
    {
        std::copy(begin, end, std::back_inserter(path_));
    }

    path(const path &right) { path_ = right.path_; }

    path &operator=(const path &right) = default;

    path(path &&right) noexcept = default;

    path &operator=(path &&right) noexcept = default;

    path(std::initializer_list<std::string> ns)
    {
        if ((ns.size() == 1) && util::contains(*ns.begin(), Sep::value)) {
            path_ = util::split(*ns.begin(), Sep::value);
        }
        else if ((ns.size() == 1) && ns.begin()->empty()) {
        }
        else
            path_ = ns;
    }

    explicit path(const std::vector<std::string> &ns)
    {
        if ((ns.size() == 1) && util::contains(*ns.begin(), Sep::value)) {
            path_ = util::split(*ns.begin(), Sep::value);
        }
        else if ((ns.size() == 1) && ns.begin()->empty()) {
        }
        else
            path_ = ns;
    }

    friend bool operator==(const path<Sep> &left, const path<Sep> &right)
    {
        return left.path_ == right.path_;
    }

    friend bool operator<(const path<Sep> &left, const path<Sep> &right)
    {
        return std::hash<path<Sep>>{}(left) < std::hash<path<Sep>>{}(right);
    }

    std::string to_string() const
    {
        return fmt::format("{}", fmt::join(path_, Sep::value));
    }

    bool is_empty() const { return path_.empty(); }

    size_t size() const { return path_.size(); }

    path operator|(const path &right) const
    {
        path res{*this};
        res.append(right);
        return res;
    }

    void operator|=(const path &right) { append(right); }

    path operator|(const std::string &right) const
    {
        path res{*this};
        res.append(right);
        return res;
    }

    void operator|=(const std::string &right) { append(right); }

    std::string &operator[](const int index) { return path_[index]; }

    const std::string &operator[](const int index) const
    {
        return path_[index];
    }

    void append(const std::string &ns) { path_.push_back(ns); }

    void append(const path &ns)
    {
        for (const auto &n : ns) {
            append(n);
        }
    }

    void pop_back()
    {
        if (!path_.empty()) {
            path_.pop_back();
        }
    }

    std::optional<path> parent() const
    {
        if (size() <= 1) {
            return {};
        }

        path res{*this};
        res.pop_back();
        return {std::move(res)};
    }

    bool starts_with(const path &right) const
    {
        return util::starts_with(path_, right.path_);
    }

    bool ends_with(const path &right) const
    {
        return util::ends_with(path_, right.path_);
    }

    path common_path(const path &right) const
    {
        path res{};
        for (auto i = 0U; i < std::min(size(), right.size()); i++) {
            if (path_[i] == right[i])
                res |= path_[i];
            else
                break;
        }
        return res;
    }

    path relative_to(const path &right) const
    {
        path res{*this};

        if (res.starts_with(right))
            util::remove_prefix(res.path_, right.path_);

        return res;
    }

    std::string relative(const std::string &name) const
    {
        if (is_empty())
            return name;

        if (name == to_string())
            return name;

        auto res = name;
        auto ns_prefix = to_string() + std::string{Sep::value};

        auto it = res.find(ns_prefix);
        while (it != std::string::npos) {
            res.erase(it, ns_prefix.size());
            it = res.find(ns_prefix);
        }

        return res;
    }

    std::string name() const
    {
        assert(size() > 0);

        return path_.back();
    }

    path::container_type::iterator begin() { return path_.begin(); }
    path::container_type::iterator end() { return path_.end(); }

    path::container_type::const_iterator cbegin() const
    {
        return path_.cbegin();
    }
    path::container_type::const_iterator cend() const { return path_.cend(); }

    path::container_type::const_iterator begin() const { return path_.begin(); }
    path::container_type::const_iterator end() const { return path_.end(); }

private:
    container_type path_;
};

} // namespace clanguml::common::model