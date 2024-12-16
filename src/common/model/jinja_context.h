/**
 * @file src/common/model/jinja_context.h
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

#include "common/model/diagram.h"
#include "common/model/diagram_element.h"
#include "common/model/element.h"
#include "common/model/element_view.h"
#include "util/error.h"

#include <inja/inja.hpp>

#include <string>

namespace clanguml::common::jinja {

/**
 * @brief Jinja diagram element context wrapper
 *
 * This template allows to have custom adl serializers for JSON, which only work
 * for Jinja templates, and not JSON generator.
 *
 * @tparam T Diagram model element
 */
template <typename T> class jinja_context {
public:
    constexpr explicit jinja_context(const T &e) noexcept
        : value{e}
    {
    }

    constexpr const T &get() const noexcept { return value; }

    template <typename U> const jinja_context<U> as() const
    {
        return jinja_context<U>(dynamic_cast<const U &>(value));
    }

    const jinja_context<T> &get_context() const { *this; }

private:
    const T &value;
};

void to_json(
    inja::json &ctx, const jinja_context<common::model::diagram_element> &jc);

void to_json(inja::json &ctx, const jinja_context<common::model::element> &jc);

void to_json(
    inja::json &ctx, const jinja_context<common::model::source_file> &jc);

std::optional<std::string> render_template(inja::Environment &env,
    const inja::json &context, const std::string &jinja_template);

std::optional<std::string> render_template(
    inja::Environment &env, const std::string &jinja_template);

} // namespace clanguml::common::jinja