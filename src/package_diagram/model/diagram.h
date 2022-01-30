/**
 * src/package_diagram/model/diagram.h
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

#include "package.h"

#include <type_safe/optional_ref.hpp>

#include <string>
#include <vector>

namespace clanguml::package_diagram::model {

class diagram : public detail::package_trait<package, std::vector> {
public:
    std::string name() const;

    void set_name(const std::string &name);

    std::string to_alias(const std::string &full_name) const;

private:
    std::string name_;

    std::vector<std::unique_ptr<package>> packages_;
};
}
