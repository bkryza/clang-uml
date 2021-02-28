/**
 * src/uml/class_diagram_model.h
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
#pragma once

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>
#include <variant>

namespace clanguml {
namespace model {
namespace class_diagram {

enum class scope_t { kPublic, kProtected, kPrivate };

enum class relationship_t {
    kNone,
    kExtension,
    kComposition,
    kAggregation,
    kContainment,
    kOwnership,
    kAssociation
};

struct element {
    std::string name;
    std::vector<std::string> namespace_;
};

struct class_element {
    scope_t scope;
    std::string name;
    std::string type;
};

struct class_member : public class_element {
    bool is_relationship{false};
    bool is_static{false};
};

struct method_argument {
    std::string type;
    std::string name;
};

struct class_method : public class_element {
    std::vector<method_argument> arguments;
    bool is_pure_virtual{false};
    bool is_virtual{false};
    bool is_const{false};
    bool is_defaulted{false};
    bool is_static{false};
};

struct class_parent {
    enum class access_t { kPublic, kProtected, kPrivate };
    std::string name;
    bool is_virtual{false};
    access_t access;
};

struct class_relationship {
    relationship_t type{relationship_t::kAssociation};
    std::string destination;
    std::string cardinality_source;
    std::string cardinality_destination;
    std::string label;
};

struct class_ : public element {
    bool is_struct{false};
    bool is_template{false};
    std::vector<class_member> members;
    std::vector<class_method> methods;
    std::vector<class_parent> bases;
    std::vector<std::string> inner_classes;

    std::vector<class_relationship> relationships;

    bool is_abstract() const
    {
        // TODO check if all base abstract methods are overriden
        // with non-abstract methods
        return std::any_of(methods.begin(), methods.end(),
            [](const auto &method) { return method.is_pure_virtual; });
    }
};

struct enum_ : public element {
    std::vector<std::string> constants;
};

struct diagram {
    std::string name;
    std::vector<class_> classes;
    std::vector<enum_> enums;
};
}
}
}
