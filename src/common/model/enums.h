/**
 * @file src/common/model/enums.h
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

#include <string>

namespace clanguml::common::model {

enum class diagram_t { kClass, kSequence, kPackage, kInclude };

enum class module_access_t { kPublic, kPrivate };
enum class access_t { kPublic, kProtected, kPrivate, kNone };

enum class relationship_t {
    kNone,
    kExtension,
    kComposition,
    kAggregation,
    kContainment,
    kOwnership,
    kAssociation,
    kInstantiation,
    kFriendship,
    kAlias,
    kDependency,
    kConstraint
};

/// Types of sequence diagram activity elements
enum class message_t {
    kCall,
    kReturn,
    kIf,
    kElse,
    kElseIf,
    kIfEnd,
    kWhile,
    kWhileEnd,
    kDo,
    kDoEnd,
    kFor,
    kForEnd,
    kTry,
    kCatch,
    kTryEnd,
    kSwitch,
    kCase,
    kSwitchEnd,
    kConditional,
    kConditionalElse,
    kConditionalEnd,
    kNone
};

/// The scope of the call expression represented in the sequence diagram
enum class message_scope_t {
    kNormal,   // This is a regular call expression
    kCondition // This is a call expression from within a control condition
               // e.g if(a->is_valid()) { ... }
};

std::string to_string(relationship_t r);

std::string to_string(access_t r);

std::string to_string(module_access_t r);

std::string to_string(message_t m);

std::string to_string(diagram_t r);

std::string to_string(message_scope_t t);

diagram_t from_string(const std::string &s);

} // namespace clanguml::common::model
