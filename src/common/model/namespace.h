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
#pragma once

#include <string>
#include <vector>

namespace clanguml::common::model {

class namespace_ {
    using container_type = std::vector<std::string>;

public:
    namespace_() = default;

    namespace_(const std::string &ns);

    namespace_(container_type::const_iterator begin,
        container_type::const_iterator end);

    namespace_(const namespace_ &right) noexcept = default;

    namespace_ &operator=(const namespace_ &right) noexcept = default;

    namespace_(namespace_ &&right) noexcept = default;

    namespace_ &operator=(namespace_ &&right) noexcept = default;

    friend bool operator==(const namespace_ &left, const namespace_ &right);

    namespace_(std::initializer_list<std::string> ns);

    explicit namespace_(const std::vector<std::string> &ns);

    std::string to_string() const;

    bool is_empty() const;

    size_t size() const;

    namespace_ operator|(const namespace_ &right) const;
    void operator|=(const namespace_ &right);
    namespace_ operator|(const std::string &right) const;
    void operator|=(const std::string &right);

    std::string &operator[](const int index);
    const std::string &operator[](const int index) const;

    void append(const std::string &ns);

    void append(const namespace_ &ns);

    void pop_back();

    bool starts_with(const namespace_ &right) const;
    bool ends_with(const namespace_ &right) const;
    namespace_ common_path(const namespace_ &right) const;
    namespace_ relative_to(const namespace_ &right) const;
    std::string relative(const std::string &name) const;
    std::string name() const;

    container_type::iterator begin();
    container_type::iterator end();
    container_type::const_iterator begin() const;
    container_type::const_iterator end() const;
    container_type::const_iterator cbegin() const;
    container_type::const_iterator cend() const;

private:
    container_type namespace_path_;
};

}