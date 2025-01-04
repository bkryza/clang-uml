/**
 * @file src/util/fmt_formatters.h
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

#include <spdlog/fmt/fmt.h>

#define MAKE_TO_STRING_FORMATTER(TYPE)                                         \
    namespace fmt {                                                            \
    template <> struct formatter<TYPE> : formatter<std::string> {              \
        template <typename FormatContext>                                      \
        auto format(                                                           \
            const TYPE &level, FormatContext &ctx) -> decltype(ctx.out())      \
        {                                                                      \
            return ::fmt::format_to(ctx.out(), "{}", to_string(level));        \
        }                                                                      \
    };                                                                         \
    }