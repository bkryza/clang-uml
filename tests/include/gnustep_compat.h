/**
 * @file tests/include/gnustep_compat.h
 *
 * Copyright (c) 2021-2026 Bartek Kryza <bkryza@gmail.com>
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

#include <GNUstepBase/GSBlocks.h>

// GNUstep Base 1.28 declares NSBackgroundActivityScheduler::_block with
// BLOCK_SCOPE (__block), which is invalid for an Objective-C instance variable.
// Preserve __block support for local variables and the remaining headers.
#pragma push_macro("BLOCK_SCOPE")
#undef BLOCK_SCOPE
#define BLOCK_SCOPE
#include <Foundation/NSBackgroundActivityScheduler.h>
#pragma pop_macro("BLOCK_SCOPE")
