/**
 * @file common/clang/USRGeneration.h
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

#if LLVM_VERSION_MAJOR == 12
#include "usr/llvm-12/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 13
#include "usr/llvm-13/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 14
#include "usr/llvm-14/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 15
#include "usr/llvm-15/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 16
#include "usr/llvm-16/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 17
#include "usr/llvm-17/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 18
#include "usr/llvm-18/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 19
#include "usr/llvm-19/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 20
#include "usr/llvm-20/USRGeneration.h"
#elif LLVM_VERSION_MAJOR == 21
#include "usr/llvm-21/USRGeneration.h"
#endif