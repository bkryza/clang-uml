/**
 * src/class_diagram/model/decorated_element.h
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

#include "enums.h"

#include "decorators/decorators.h"
#include "inja/inja.hpp"

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace clanguml::common::model {

using comment_t = inja::json;

class decorated_element {
public:
    bool skip() const;

    bool skip_relationship() const;

    std::pair<relationship_t, std::string> get_relationship() const;

    std::string style_spec() const;

    const std::vector<std::shared_ptr<decorators::decorator>> &
    decorators() const;

    void add_decorators(
        const std::vector<std::shared_ptr<decorators::decorator>> &decorators);

    void append(const decorated_element &de);

    std::optional<comment_t> comment() const;

    void set_comment(const comment_t &c);

private:
    std::vector<std::shared_ptr<decorators::decorator>> decorators_;
    std::optional<comment_t> comment_;
};

} // namespace clanguml::common::model
