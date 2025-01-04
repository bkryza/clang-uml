/**
 * @file src/util/logging.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include "logging.h"

#include "util.h"

#include <string>

namespace clanguml::logging {

logger_type_t logger_type(logger_type_t type)
{
    static logger_type_t logger_type_singleton_{logger_type_t::text};

    if (type != logger_type_t::get) {
        logger_type_singleton_ = type;
    }

    return logger_type_singleton_;
}

std::string to_string(logger_type_t type)
{
    switch (type) {
    case logger_type_t::text:
        return "text";
    case logger_type_t::json:
        return "json";
    case logger_type_t::get:
        return "get";
    default:
        return "<unknown>";
    }
}

std::string to_string(spdlog::level::level_enum level)
{
    switch (level) {
    case spdlog::level::level_enum::critical:
        return "critical";
    case spdlog::level::level_enum::debug:
        return "debug";
    case spdlog::level::level_enum::err:
        return "err";
    case spdlog::level::level_enum::info:
        return "info";
    case spdlog::level::level_enum::trace:
        return "trace";
    case spdlog::level::level_enum::warn:
        return "warn";
    default:
        return "<unknown>";
    }
}

void escape_json_string(std::string &s)
{
    clanguml::util::replace_all(s, "\\", "\\\\");
    clanguml::util::replace_all(s, "\"", "\\\"");
    clanguml::util::replace_all(s, "\n", "");
    clanguml::util::replace_all(s, "\r", "");
    clanguml::util::replace_all(s, "\t", " ");
    clanguml::util::replace_all(s, "\b", " ");
}
} // namespace clanguml::logging