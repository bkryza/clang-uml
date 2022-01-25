/**
 * src/package_diagram/model/visitor/element_visitor_context.cc
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

#include "element_visitor_context.h"

#include "translation_unit_context.h"

namespace clanguml::package_diagram::visitor {

template <typename T>
element_visitor_context<T>::element_visitor_context(
    clanguml::package_diagram::model::diagram &diagram, T &element)
    : element_{element}
    , diagram_{diagram}
{
}

template <typename T> T &element_visitor_context<T>::element()
{
    return element_;
}

template <typename T>
clanguml::package_diagram::model::diagram &element_visitor_context<T>::diagram()
{
    return diagram_;
}
}