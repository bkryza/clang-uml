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

struct diagram_context_tag;
struct element_context_tag;

/**
 * @brief Jinja diagram element context wrapper
 *
 * This template allows to have custom adl serializers for JSON, which only work
 * for Jinja templates, and not JSON generator.
 *
 * @tparam T Diagram model element
 */
template <typename T, typename Tag> class jinja_context {
public:
    explicit jinja_context(
        const T &e, const inja::json &diagram_context) noexcept
        : value_{e}
        , diagram_context_{diagram_context}
    {
    }

    explicit jinja_context(const T &e) noexcept
        : jinja_context(e, {})
    {
    }

    const T &get() const noexcept { return value_; }

    template <typename U> const jinja_context<U, Tag> as() const
    {
        return jinja_context<U, Tag>(
            dynamic_cast<const U &>(value_), diagram_context_);
    }

    const inja::json &diagram_context() const { return diagram_context_; }

private:
    const T &value_;
    const inja::json &diagram_context_;
};

template <typename T>
using element_context = jinja_context<T, element_context_tag>;
template <typename T>
using diagram_context = jinja_context<T, diagram_context_tag>;

void to_json(inja::json &ctx,
    const element_context<common::model::decorated_element> &jc);

void to_json(
    inja::json &ctx, const element_context<common::model::diagram_element> &jc);

void to_json(
    inja::json &ctx, const element_context<common::model::element> &jc);

void to_json(
    inja::json &ctx, const element_context<common::model::source_file> &jc);

void to_json(
    inja::json &ctx, const element_context<common::model::source_location> &jc);

void to_json(inja::json &ctx,
    const diagram_context<common::model::decorated_element> &jc);

void to_json(
    inja::json &ctx, const diagram_context<common::model::diagram_element> &jc);

void to_json(
    inja::json &ctx, const diagram_context<common::model::element> &jc);

void to_json(
    inja::json &ctx, const diagram_context<common::model::source_file> &jc);

void to_json(
    inja::json &ctx, const diagram_context<common::model::source_location> &jc);

std::optional<std::string> render_template(inja::Environment &env,
    const inja::json &context, const std::string &jinja_template);

std::optional<std::string> render_template(
    inja::Environment &env, const std::string &jinja_template);

} // namespace clanguml::common::jinja