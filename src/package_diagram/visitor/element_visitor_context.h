/**
 * src/package_diagram/model/visitor/element_visitor_context.h
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

#include "package_diagram/model/diagram.h"

namespace clanguml::package_diagram::visitor {

class translation_unit_context;

template <typename T> class element_visitor_context {
public:
    element_visitor_context(
        clanguml::package_diagram::model::diagram &diagram, T &element);

    T &element();

    clanguml::package_diagram::model::diagram &diagram();

private:
    translation_unit_context *ctx_;

    T &element_;
    clanguml::package_diagram::model::diagram &diagram_;
};

}
