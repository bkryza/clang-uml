/**
 * @file src/common/model/path.h
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

#include <optional>
#include <string>
#include <vector>

namespace clanguml::common::model {

/**
 * @brief Type of diagram path
 *
 * Paths in diagrams represent the nest structure within a diagram, e.g.
 * a nested set of namespaces or nested set of directories.
 */
enum class path_type {
    kNamespace,  /*!< Namespace path */
    kFilesystem, /*!< Filesystem path */
    kModule      /*!< Module path */
};

std::string to_string(path_type pt);

/**
 * @brief Diagram path
 *
 * This class stores a diagram path, such as a namespace or directory
 * structure.
 */
class path {

    /**
     * Returns the path separator based on the path type.
     *
     * @return Path separator
     */
    static const char *separator(path_type pt)
    {
        switch (pt) {
        case path_type::kNamespace:
            return "::";
        case path_type::kModule:
            return ".";
        case path_type::kFilesystem:
#ifdef _WIN32
            return "\\";
#else
            return "/";
#endif
        }

        return "::";
    }

    /**
     * Returns the path separator based on the type of the instance path.
     *
     * @return Path separator
     */
    const char *separator() const { return separator(path_type_); }

public:
    using container_type = std::vector<std::string>;

    static container_type split(
        const std::string &ns, path_type pt = path_type::kNamespace)
    {
        container_type result;
        if (pt == path_type::kModule) {
            auto path_toks = util::split(ns, separator(pt));
            for (const auto &pt : path_toks) {
                const auto subtoks = util::split(pt, ":");
                if (subtoks.size() == 2) {
                    result.push_back(subtoks.at(0));
                    result.push_back(fmt::format(":{}", subtoks.at(1)));
                }
                else
                    result.push_back(subtoks.at(0));
            }
        }
        else
            result = util::split(ns, separator(pt));

        return result;
    }

    path(path_type pt = path_type::kNamespace)
        : path_type_{pt}
    {
    }

    path(const std::string &ns, path_type pt = path_type::kNamespace)
        : path_type_{pt}
    {
        if (ns.empty())
            return;

        path_ = split(ns, pt);
    }

    virtual ~path() = default;

    path(container_type::const_iterator begin,
        container_type::const_iterator end,
        path_type pt = path_type::kNamespace)
        : path(pt)
    {
        if (begin == end)
            return;

        std::copy(begin, end, std::back_inserter(path_));
    }

    path(const path &right) = default;

    path &operator=(const path &right)
    {
        if (&right == this)
            return *this;

        if (path_type_ != right.path_type_)
            throw std::runtime_error(
                "Cannot assign a path to a path with another path type.");

        path_type_ = right.path_type_;
        path_ = right.path_;

        return *this;
    }

    path(path &&right) noexcept = default;

    path &operator=(path &&right) noexcept = default;

    path(std::initializer_list<std::string> ns,
        path_type pt = path_type::kNamespace)
        : path(pt)
    {
        if ((ns.size() == 1) &&
            util::contains(*ns.begin(), std::string{separator()})) {
            path_ = util::split(*ns.begin(), separator());
        }
        else if ((ns.size() == 1) && ns.begin()->empty()) {
        }
        else
            path_ = ns;
    }

    explicit path(const std::vector<std::string> &ns,
        path_type pt = path_type::kNamespace)
        : path(pt)
    {
        if ((ns.size() == 1) &&
            util::contains(*ns.begin(), std::string{separator()})) {
            path_ = util::split(*ns.begin(), separator());
        }
        else if ((ns.size() == 1) && ns.begin()->empty()) {
        }
        else
            path_ = ns;
    }

    friend bool operator==(const path &left, const path &right)
    {
        return left.path_ == right.path_;
    }

    friend bool operator<(const path &left, const path &right)
    {
        return left.to_string() < right.to_string();
    }

    /**
     * Render the path as string.
     *
     * @return  String representation of the path.
     */
    std::string to_string() const
    {
        auto result =
            fmt::format("{}", fmt::join(path_, std::string{separator()}));

        if (path_type_ == path_type::kModule) {
            util::replace_all(result, ".:", ":");
        }

        return result;
    }

    /**
     * Whether the path is empty.
     *
     * @return
     */
    bool is_empty() const { return path_.empty(); }

    /**
     * Return the number of elements in the path.
     *
     * @return Size of path.
     */
    size_t size() const { return path_.size(); }

    /**
     * Append path to path.
     *
     * @param right Path to append at the end.
     * @return New merged path.
     */
    path operator|(const path &right) const
    {
        path res{*this};
        res.path_type_ = right.path_type_;
        res.append(right);
        return res;
    }

    /**
     * Append path to the current path.
     *
     * @param right
     */
    void operator|=(const path &right) { append(right); }

    /**
     * Append path element to path.
     *
     * @return New path.
     */
    path operator|(const std::string &right) const
    {
        path res{*this};
        res.append(right);
        return res;
    }

    /**
     * Append path element to the current path.
     *
     * @param right Path element to append.
     */
    void operator|=(const std::string &right) { append(right); }

    std::string &operator[](const unsigned int index) { return path_[index]; }

    const std::string &operator[](const unsigned int index) const
    {
        return path_[index];
    }

    /**
     * Append path element to path.
     *
     * @return New path.
     */
    void append(const std::string &name) { path_.push_back(name); }

    /**
     * Append path to current path.
     *
     * @param ns Path to append.
     */
    void append(const path &ns)
    {
        for (const auto &n : ns) {
            append(n);
        }
    }

    /**
     * Drop the last element of the path.
     */
    void pop_back()
    {
        if (!path_.empty()) {
            path_.pop_back();
        }
    }

    /**
     * Get the parent of the last element in the path.
     *
     * @return Path to the parent of the last element, or nullopt.
     */
    std::optional<path> parent() const
    {
        if (size() <= 1) {
            return {};
        }

        path res{*this};
        res.pop_back();
        return {std::move(res)};
    }

    /**
     * Returns true if path starts with specified prefix.
     * @param prefix Path prefix to check.
     * @return
     */
    bool starts_with(const path &prefix) const
    {
        return util::starts_with(path_, prefix.path_);
    }

    /**
     * Returns true if path ends with suffix
     * @param suffix Path suffix to check
     * @return
     */
    bool ends_with(const path &suffix) const
    {
        return util::ends_with(path_, suffix.path_);
    }

    /**
     * @brief Returns the common prefix of 2 paths.
     *
     * If no common prefix exists between 2 paths, the result is an empty path.
     *
     * @param right Path to compare
     * @return Common path prefix
     */
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

    /**
     * Make the current path relative to the other path, if possible.
     *
     * If not, return the original path.
     *
     * @param right Parent path
     * @return Path relative to `right`
     */
    path relative_to(const path &right) const
    {
        path res{*this};

        if (res.starts_with(right))
            util::remove_prefix(res.path_, right.path_);

        return res;
    }

    /**
     * Make path represented as a string relative to the current path.
     *
     * @param ns Path to make relative against *this.
     * @return Relative path.
     */
    std::string relative(const std::string &ns) const
    {
        if (is_empty())
            return ns;

        if (ns == to_string())
            return ns;

        auto res = ns;
        auto ns_prefix = to_string() + std::string{separator()};

        auto it = res.find(ns_prefix);
        while (it != std::string::npos) {
            res.erase(it, ns_prefix.size());
            it = res.find(ns_prefix);
        }

        return res;
    }

    /**
     * Return the name of the last element in the path.
     *
     * @return Name of the last element in the path.
     */
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

    /**
     * Get path type.
     *
     * @return Path type.
     */
    path_type type() const { return path_type_; }

    const container_type &tokens() const { return path_; }

private:
    path_type path_type_;
    container_type path_;
};

} // namespace clanguml::common::model