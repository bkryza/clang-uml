/**
 * src/uml/decorators.h
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

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace clanguml {
namespace decorators {
struct decorator_toks {
    std::string label;
    std::vector<std::string> diagrams;
    std::string param;
    std::string text;
};

struct decorator {
    std::vector<std::string> diagrams;

    virtual ~decorator() = default;

    static std::shared_ptr<decorator> from_string(std::string_view c);

    bool applies_to_diagram(std::string name);

protected:
    decorator_toks tokenize(const std::string &label, std::string_view c);
};

struct note : public decorator {
    static const std::string label;

    std::string position{"left"};
    std::string text;

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

struct skip : public decorator {
    static const std::string label;

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

struct skip_relationship : public decorator {
    static const std::string label;

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

struct style : public decorator {
    static const std::string label;

    std::string spec;
    static std::shared_ptr<decorator> from_string(std::string_view c);
};

struct aggregation : public decorator {
    static const std::string label;

    std::string multiplicity;
    static std::shared_ptr<decorator> from_string(std::string_view c);
};

std::vector<std::shared_ptr<decorator>> parse(
    std::string documentation_block, std::string clanguml_tag = "uml");

} // namespace decorators
} // namespace clanguml
