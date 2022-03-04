/**
 * src/common/model/namespace.h
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

#include "namespace.h"

#include "util/util.h"

namespace clanguml::common::model {

namespace_::namespace_(std::initializer_list<std::string> ns)
{
    if ((ns.size() == 1) && util::contains(*ns.begin(), "::"))
        namespace_path_ = util::split(*ns.begin(), "::");
    else
        namespace_path_ = ns;
}

namespace_::namespace_(const std::vector<std::string> &ns)
{
    if ((ns.size() == 1) && util::contains(*ns.begin(), "::"))
        namespace_path_ = util::split(*ns.begin(), "::");
    else
        namespace_path_ = ns;
}

namespace_::namespace_(const std::string &ns)
{
    namespace_path_ = util::split(ns, "::");
}

namespace_::namespace_(
    container_type::const_iterator begin, container_type::const_iterator end)
{
    std::copy(begin, end, std::back_inserter(namespace_path_));
}

std::string namespace_::to_string() const
{
    return fmt::format("{}", fmt::join(namespace_path_, "::"));
}

size_t namespace_::size() const { return namespace_path_.size(); }

bool namespace_::is_empty() const { return namespace_path_.empty(); }

namespace_::container_type::iterator namespace_::begin()
{
    return namespace_path_.begin();
}
namespace_::container_type::iterator namespace_::end()
{
    return namespace_path_.end();
}

namespace_::container_type::const_iterator namespace_::cbegin() const
{
    return namespace_path_.cbegin();
}
namespace_::container_type::const_iterator namespace_::cend() const
{
    return namespace_path_.cend();
}

namespace_::container_type::const_iterator namespace_::begin() const
{
    return namespace_path_.begin();
}
namespace_::container_type::const_iterator namespace_::end() const
{
    return namespace_path_.end();
}

void namespace_::append(const std::string &ns)
{
    namespace_path_.push_back(ns);
}

void namespace_::append(const namespace_ &ns)
{
    for (const auto &n : ns) {
        append(n);
    }
}

void namespace_::pop_back() { namespace_path_.pop_back(); }

namespace_ namespace_::operator|(const namespace_ &right) const
{
    namespace_ res{*this};
    res.append(right);
    return res;
}

void namespace_::operator|=(const namespace_ &right) { append(right); }

namespace_ namespace_::operator|(const std::string &right) const
{
    namespace_ res{*this};
    res.append(right);
    return res;
}

void namespace_::operator|=(const std::string &right) { append(right); }

std::string &namespace_::operator[](const int index)
{
    return namespace_path_[index];
}

const std::string &namespace_::operator[](const int index) const
{
    return namespace_path_[index];
}

bool namespace_::starts_with(const namespace_ &right) const
{
    return util::starts_with(namespace_path_, right.namespace_path_);
}

namespace_ namespace_::common_path(const namespace_ &right) const
{
    namespace_ res{};
    for (auto i = 0U; i < std::min(size(), right.size()); i++) {
        if (namespace_path_[i] == right[i])
            res |= namespace_path_[i];
        else
            break;
    }
    return res;
}

namespace_ namespace_::relative_to(const namespace_ &right) const
{
    namespace_ res{*this};

    if (res.starts_with(right))
        util::remove_prefix(res.namespace_path_, right.namespace_path_);

    return res;
}

std::string namespace_::relative(const std::string &name) const
{
    /*
    std::vector<std::string> namespaces_sorted{namespaces};

    std::sort(namespaces_sorted.rbegin(), namespaces_sorted.rend());

    auto res = name;

    for (const auto &ns : namespaces_sorted) {
        if (ns.empty())
            continue;

        if (name == ns)
            return split(n, "::").back();

        auto ns_prefix = ns + "::";
        auto it = res.find(ns_prefix);
        while (it != std::string::npos) {
            res.erase(it, ns_prefix.size());
            it = res.find(ns_prefix);
        }
    }
    return res;
     */

    if (is_empty())
        return name;

    if (name == to_string())
        return name;

    auto res = name;
    auto ns_prefix = to_string() + "::";

    auto it = res.find(ns_prefix);
    while (it != std::string::npos) {
        res.erase(it, ns_prefix.size());
        it = res.find(ns_prefix);
    }

    return res;
}

bool operator==(const namespace_ &left, const namespace_ &right)
{
    return left.namespace_path_ == right.namespace_path_;
}

std::string namespace_::name() const
{
    assert(size() > 0);

    return namespace_path_.back();
}

}