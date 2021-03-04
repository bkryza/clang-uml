/**
 * src/cx/util.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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

#include "cx/util.h"

#include <spdlog/spdlog.h>

namespace clanguml {
namespace cx {
namespace util {

std::string to_string(CXString &&cxs)
{
    std::string r{clang_getCString(cxs)};
    clang_disposeString(cxs);
    return r;
}
} // namespace util
} // namespace cx
} // namespace clanguml
