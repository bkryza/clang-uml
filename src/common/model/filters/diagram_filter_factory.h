/**
 * @file src/common/model/filters/diagram_filter_factory.h
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

#include "diagram_filter.h"
#include "package_diagram/model/diagram.h"

namespace clanguml::common::model {

struct basic_diagram_filter_initializer {
    void initialize(const config::diagram &c, diagram_filter &df);
};

struct advanced_diagram_filter_initializer {
    void initialize(const config::diagram &c, diagram_filter &df);
};

class diagram_filter_factory {
public:
    static std::unique_ptr<diagram_filter> create(
        const common::model::diagram &d, const config::diagram &c)
    {
        auto filter = std::make_unique<diagram_filter>(
            d, c, diagram_filter::private_constructor_tag_t{});

        basic_diagram_filter_initializer init;

        init.initialize(c, *filter);

        return filter;
    }
};

} // namespace clanguml::common::model