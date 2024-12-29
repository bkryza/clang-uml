/**
 * @file src/common/model/jinja_context.cc
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

#include "jinja_context.h"

#include "common/generators/display_adapters.h"

namespace clanguml::common::jinja {

using clanguml::common::generators::display_name_adapter;

using namespace clanguml::common::model;

void to_json(inja::json &ctx,
    const element_context<common::model::decorated_element> &jc)
{
    if (const auto maybe_comment = jc.get().comment();
        maybe_comment.has_value()) {
        ctx["element"]["comment"] = maybe_comment.value();
    }
}

void to_json(inja::json &ctx, const element_context<diagram_element> &jc)
{
    to_json(ctx, jc.as<decorated_element>());

    ctx["element"]["name"] = display_name_adapter(jc.get()).name();
    ctx["element"]["type"] = jc.get().type_name();
    ctx["element"]["alias"] = jc.get().alias();
    ctx["element"]["full_name"] =
        display_name_adapter(jc.get()).full_name(false);
    auto maybe_doxygen_link = jc.get().doxygen_link();
    if (maybe_doxygen_link)
        ctx["element"]["doxygen_link"] = maybe_doxygen_link.value();

    if (jc.diagram_context().contains("git")) {
        ctx["git"] = jc.diagram_context()["git"];
    }

    if (jc.diagram_context().contains("user_data")) {
        ctx["user_data"] = jc.diagram_context()["user_data"];
    }

    to_json(ctx, jc.as<source_location>());
}

void to_json(inja::json &ctx, const element_context<element> &jc)
{
    to_json(ctx, jc.as<diagram_element>());

    ctx["element"]["using_namespace"] = jc.get().using_namespace().to_string();
    ctx["element"]["namespace"] = jc.get().get_namespace().to_string();
}

void to_json(inja::json &ctx, const element_context<source_file> &jc)
{
    to_json(ctx, jc.as<diagram_element>());

    std::filesystem::path fullNamePath{
        ctx["element"]["full_name"].get<std::string>()};
    fullNamePath.make_preferred();
    ctx["element"]["full_name"] = fullNamePath.string();
}

void to_json(inja::json &ctx, const diagram_context<source_location> &jc)
{
    const auto &e = jc.get();

    if (!e.file().empty()) {
        const std::filesystem::path file{e.file()};
        std::string git_relative_path = file.string();
        if (!e.file_relative().empty()) {
#if _MSC_VER
            if (file.is_absolute() && ctx.contains("git")) {
#else
            if (file.is_absolute() && ctx.contains("git")) {
#endif
                git_relative_path =
                    std::filesystem::relative(file, ctx["git"]["toplevel"])
                        .string();
                ctx["source"]["path"] = util::path_to_url(git_relative_path);
            }
            else {
                ctx["source"]["path"] = e.file();
            }
        }
        else {
            git_relative_path = "";
            ctx["source"]["path"] = e.file();
        }

        ctx["source"]["full_path"] = file.string();
        ctx["source"]["name"] = file.filename().string();
        ctx["source"]["line"] = e.line();
    }
}

void to_json(inja::json &ctx, const element_context<source_location> &jc)
{
    const auto &e = jc.get();

    if (!e.file().empty()) {
        const std::filesystem::path file{e.file()};
        std::string git_relative_path = file.string();
        if (!e.file_relative().empty()) {
#if _MSC_VER
            if (file.is_absolute() && ctx.contains("git")) {
#else
            if (file.is_absolute() && ctx.contains("git")) {
#endif
                git_relative_path =
                    std::filesystem::relative(file, ctx["git"]["toplevel"])
                        .string();
                ctx["element"]["source"]["path"] =
                    util::path_to_url(git_relative_path);
            }
            else {
                ctx["element"]["source"]["path"] = e.file();
            }
        }
        else {
            git_relative_path = "";
            ctx["element"]["source"]["path"] = e.file();
        }

        ctx["element"]["source"]["full_path"] = file.string();
        ctx["element"]["source"]["name"] = file.filename().string();
        ctx["element"]["source"]["line"] = e.line();
    }
}

void to_json(inja::json &ctx,
    const diagram_context<common::model::decorated_element> &jc)
{
    if (const auto maybe_comment = jc.get().comment();
        maybe_comment.has_value()) {
        ctx["comment"] = maybe_comment.value();
    }
}

void to_json(
    inja::json &ctx, const diagram_context<common::model::diagram_element> &jc)
{
    to_json(ctx, jc.as<decorated_element>());

    ctx["name"] = display_name_adapter(jc.get()).name();
    ctx["type"] = jc.get().type_name();
    ctx["alias"] = jc.get().alias();
    ctx["full_name"] = display_name_adapter(jc.get()).full_name(false);
    auto maybe_doxygen_link = jc.get().doxygen_link();
    if (maybe_doxygen_link)
        ctx["doxygen_link"] = maybe_doxygen_link.value();

    to_json(ctx, jc.as<source_location>());
}

void to_json(inja::json &ctx, const diagram_context<common::model::element> &jc)
{
    to_json(ctx, jc.as<diagram_element>());

    ctx["using_namespace"] = jc.get().using_namespace().to_string();
    ctx["namespace"] = jc.get().get_namespace().to_string();
    if (const auto maybe_comment = jc.get().comment();
        maybe_comment.has_value()) {
        ctx["comment"] = maybe_comment.value();
    }
}

void to_json(inja::json &ctx, const diagram_context<source_file> &jc)
{
    to_json(ctx, jc.as<diagram_element>());

    std::filesystem::path fullNamePath{ctx["full_name"].get<std::string>()};
    fullNamePath.make_preferred();
    ctx["full_name"] = fullNamePath.string();
}

std::optional<std::string> render_template(inja::Environment &env,
    const inja::json &context, const std::string &jinja_template)
{
    std::optional<std::string> result;

    if (jinja_template.empty())
        return result;

    try {
        // Render the directive with template engine first
        auto rendered_template =
            env.render(std::string_view{jinja_template}, context);

        result = std::move(rendered_template);
    }
    catch (const clanguml::error::uml_alias_missing &e) {
        LOG_WARN("Failed to render Jinja template '{}' due to unresolvable "
                 "alias: {}",
            jinja_template, e.what());
    }
    catch (const inja::json::parse_error &e) {
        LOG_WARN("Failed to parse Jinja template: {}", jinja_template);
    }
    catch (const inja::json::exception &e) {
        LOG_WARN("Failed to render Jinja template: \n{}\n due to: {}",
            jinja_template, e.what());
    }
    catch (const std::regex_error &e) {
        LOG_WARN("Failed to render Jinja template: \n{}\n due to "
                 "std::regex_error: {}",
            jinja_template, e.what());
    }
    catch (const std::exception &e) {
        LOG_WARN("Failed to render Jinja template: \n{}\n due to: {}",
            jinja_template, e.what());
    }

    return result;
}

std::optional<std::string> render_template(
    inja::Environment &env, const std::string &jinja_template)
{
    inja::json empty;
    return render_template(env, empty, jinja_template);
}

} // namespace clanguml::common::jinja