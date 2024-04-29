/**
 * @file src/decorators/decorators.h
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

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace clanguml {
namespace decorators {
struct decorator_toks {
    std::string label;
    std::vector<std::string> diagrams;
    std::string param;
    std::string text;
};

/**
 * @brief Base class for clang-uml comment tags
 *
 * This class provides basic interface for `clang-uml` comment tags, called
 * decorators. All decorators are added to the code using `uml` Doxygen-style
 * tag.
 */
struct decorator {
    /** List of diagram names to which a given decorator applies */
    std::vector<std::string> diagrams;

    virtual ~decorator() = default;

    /**
     * @brief Create decorator of specific type based on it's string
     *        representation.
     * @param c Decorator string representation extracted from the comment
     * @return Decorator instance
     */
    static std::shared_ptr<decorator> from_string(std::string_view c);

    /**
     * @brief Check if decorator applies to a specific diagram.
     *
     * @param name Name of the diagram
     * @return True, if this decorator applies to diagram `name`
     */
    bool applies_to_diagram(const std::string &name);

protected:
    decorator_toks tokenize(const std::string &label, std::string_view c);
};

/**
 * @brief Represents a note diagram element
 */
struct note : public decorator {
    static inline const std::string label{"note"};

    std::string position{"left"};
    std::string text;

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

/**
 * @brief Whether a decorated element should be skipped from a diagram
 */
struct skip : public decorator {
    static inline const std::string label{"skip"};

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

/**
 * @brief Whether a decorated relationships should be skipped from a diagram
 */
struct skip_relationship : public decorator {
    static inline const std::string label{"skiprelationship"};

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

/**
 * @brief Apply specific style to a decorated diagram element
 */
struct style : public decorator {
    static inline const std::string label{"style"};

    std::string spec;
    static std::shared_ptr<decorator> from_string(std::string_view c);
};

/**
 * @brief Base class for decorators overriding default relationship types
 */
struct relationship : public decorator {
    std::string multiplicity;
};

/**
 * @brief Make a member an aggregation relationship
 */
struct aggregation : public relationship {
    static inline const std::string label{"aggregation"};

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

/**
 * @brief Make a member a composition relationship
 */
struct composition : public relationship {
    static inline const std::string label{"composition"};

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

/**
 * @brief Make a member an association relationship
 */
struct association : public relationship {
    static inline const std::string label{"association"};

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

/**
 * @brief Represents a call message in sequence diagram
 */
struct call : public decorator {
    static inline const std::string label{"call"};

    std::string callee;

    static std::shared_ptr<decorator> from_string(std::string_view c);
};

/**
 * @brief Parse a documentation block and extract all clang-uml decorators
 *
 * @param documentation_block Documentation block extracted from source code
 * @param clanguml_tag Name of the clanguml tag (default `uml`)
 * @return Pair of: a list of clang-uml decorators extracted from comment and
 *                  a comment stripped of any uml directives
 */
std::pair<std::vector<std::shared_ptr<decorator>>, std::string> parse(
    std::string documentation_block, const std::string &clanguml_tag = "uml");

} // namespace decorators
} // namespace clanguml
