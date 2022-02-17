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
#include "common/model/nested_trait.h"
#include "common/model/stylable_element.h"
#include "util/util.h"

#include <spdlog/spdlog.h>
#include <type_safe/optional_ref.hpp>

#include <set>
#include <string>
#include <vector>

namespace clanguml::package_diagram::model {

class package : public common::model::element,
                public common::model::stylable_element,
                public common::model::nested_trait<package> {
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
