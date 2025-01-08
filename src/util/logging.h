/**
 * @file src/util/logging.h
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
#pragma once

#include "fmt_formatters.h"

#include <inja/inja.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// For release builds, use only file names in the log paths, for debug use
// full paths to make it easier to navigate to specific file:line in the code
// from logs
#if defined(NDEBUG)
#define FILENAME_                                                              \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#else
#define FILENAME_ __FILE__
#endif

#define LOG_ERROR(fmt__, ...)                                                  \
    ::clanguml::logging::log_impl(                                             \
        spdlog::level::err, fmt__, FILENAME_, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(fmt__, ...)                                                   \
    ::clanguml::logging::log_impl(                                             \
        spdlog::level::warn, fmt__, FILENAME_, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(fmt__, ...)                                                   \
    ::clanguml::logging::log_impl(                                             \
        spdlog::level::info, fmt__, FILENAME_, __LINE__, ##__VA_ARGS__)

#define LOG_DBG(fmt__, ...)                                                    \
    ::clanguml::logging::log_impl(                                             \
        spdlog::level::debug, fmt__, FILENAME_, __LINE__, ##__VA_ARGS__)

#define LOG_TRACE(fmt__, ...)                                                  \
    ::clanguml::logging::log_impl(                                             \
        spdlog::level::trace, fmt__, FILENAME_, __LINE__, ##__VA_ARGS__)

namespace fmt {
template <> struct formatter<inja::json> : formatter<std::string> {
    auto format(const inja::json &json,
        format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", json.dump());
    }
};
} // namespace fmt

namespace clanguml::logging {

enum class logger_type_t {
    text,
    json,
    get /*!< This key is used to get the global logger type value */
};

logger_type_t logger_type(logger_type_t type = logger_type_t::get);

std::string to_string(logger_type_t type);

std::string to_string(spdlog::level::level_enum level);

void escape_json_string(std::string &s);

template <typename T> decltype(auto) escape_json(T &&val)
{
    using DecayedT = std::decay_t<T>;
    if constexpr (std::is_same_v<DecayedT, inja::json>) {
        std::string result{val.dump()};
        return result;
    }
    else if constexpr (std::is_convertible_v<DecayedT, std::string>) {
        std::string result{val};
        escape_json_string(result);
        return result;
    }
    else
        return std::forward<T>(val);
}

template <typename FilenameT, typename LineT, typename... Args>
void log_impl(spdlog::level::level_enum level, logger_type_t type,
    std::string_view fmt_, FilenameT f, LineT l, Args &&...args)
{
    if (type == logger_type_t::text) {
        spdlog::get("clanguml-logger")
            ->log(level, fmt::runtime("[{}:{}] " + std::string{fmt_}), f, l,
                std::forward<Args>(args)...);
    }
    else {
        spdlog::get("clanguml-logger")
            ->log(level,
                fmt::runtime(R"("file": "{}", "line": {}, "message": ")" +
                    std::string{fmt_} + "\""),
                f, l, escape_json(std::forward<Args>(args))...);
    }
}

template <typename FilenameT, typename LineT, typename... Args>
void log_impl(spdlog::level::level_enum level, std::string_view fmt_,
    FilenameT f, LineT t, Args &&...args)
{
    log_impl(level, logger_type(), fmt_, f, t, std::forward<Args>(args)...);
}

} // namespace clanguml::logging