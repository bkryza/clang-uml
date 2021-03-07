/**
 * tests/test_cases.h
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

#include "config/config.h"
#include "cx/compilation_database.h"
#include "puml/class_diagram_generator.h"
#include "puml/sequence_diagram_generator.h"
#include "uml/class_diagram_model.h"
#include "uml/class_diagram_visitor.h"
#include "uml/sequence_diagram_visitor.h"
#include "util/util.h"

#include "catch.h"

#include <complex>
#include <filesystem>
#include <string>

using Catch::Matchers::Contains;
using Catch::Matchers::EndsWith;
using Catch::Matchers::StartsWith;
using Catch::Matchers::VectorContains;
using clanguml::cx::compilation_database;
using namespace clanguml::util;

std::pair<clanguml::config::config, compilation_database> load_config(
    const std::string &test_name);

clanguml::model::sequence_diagram::diagram generate_sequence_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram);

clanguml::model::class_diagram::diagram generate_class_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram);

std::string generate_sequence_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::sequence_diagram::diagram &model);

std::string generate_class_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::class_diagram::diagram &model);

void save_puml(const std::string &path, const std::string &puml);

namespace clanguml {
namespace test {
namespace matchers {

using Catch::CaseSensitive;
using Catch::Matchers::StdString::CasedString;
using Catch::Matchers::StdString::ContainsMatcher;

struct Public {
    Public(std::string const &method)
        : m_method{method}
    {
    }

    operator std::string() const { return "+" + m_method; }

    std::string m_method;
};

struct Protected {
    Protected(std::string const &method)
        : m_method{method}
    {
    }

    operator std::string() const { return "#" + m_method; }

    std::string m_method;
};

struct Private {
    Private(std::string const &method)
        : m_method{method}
    {
    }

    operator std::string() const { return "-" + m_method; }

    std::string m_method;
};

struct Abstract {
    Abstract(std::string const &method)
        : m_method{method}
    {
    }

    operator std::string() const { return "{abstract} " + m_method; }

    std::string m_method;
};

struct Static {
    Static(std::string const &method)
        : m_method{method}
    {
    }

    operator std::string() const { return "{static} " + m_method; }

    std::string m_method;
};

struct Const {
    Const(std::string const &method)
        : m_method{method}
    {
    }

    operator std::string() const { return m_method; }

    std::string m_method;
};

struct Default {
    Default(std::string const &method)
        : m_method{method}
    {
    }

    operator std::string() const { return m_method; }

    std::string m_method;
};

struct HasCallWithResultMatcher : ContainsMatcher {
    HasCallWithResultMatcher(
        CasedString const &comparator, CasedString const &resultComparator)
        : ContainsMatcher(comparator)
        , m_resultComparator{resultComparator}
    {
    }

    bool match(std::string const &source) const override
    {
        return Catch::contains(
                   m_comparator.adjustString(source), m_comparator.m_str) &&
            Catch::contains(
                m_comparator.adjustString(source), m_resultComparator.m_str);
    }

    CasedString m_resultComparator;
};

ContainsMatcher HasCall(std::string const &from, std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("\"{}\" -> \"{}\" : {}()", from, from, message),
            caseSensitivity));
}

ContainsMatcher HasCall(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("\"{}\" -> \"{}\" : {}()", from, to, message),
            caseSensitivity));
}

auto HasCallWithResponse(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCallWithResultMatcher(
        CasedString(fmt::format("\"{}\" -> \"{}\" : {}()", from, to, message),
            caseSensitivity),
        CasedString(
            fmt::format("\"{}\" --> \"{}\"", to, from), caseSensitivity));
}

struct AliasMatcher {
    AliasMatcher(const std::string &puml_)
        : puml{split(puml_, "\n")}
    {
    }

    std::string operator()(const std::string &name)
    {
        std::vector<std::string> patterns;
        patterns.push_back("class \"" + name + "\" as ");
        patterns.push_back("abstract \"" + name + "\" as ");
        patterns.push_back("enum \"" + name + "\" as ");

        for (const auto &line : puml) {
            for (const auto &pattern : patterns) {
                const auto idx = line.find(pattern);
                if (idx != std::string::npos) {
                    std::string res = line.substr(idx + pattern.size());
                    return trim(res);
                }
            }
        }

        throw std::runtime_error(fmt::format(
            "Cannot find alias {} in {}", name, fmt::join(puml, "\n")));
    }

    const std::vector<std::string> puml;
};

ContainsMatcher IsClass(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("class " + str, caseSensitivity));
}

ContainsMatcher IsClassTemplate(std::string const &str,
    std::string const &tmplt,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("class \"{}<{}>\"", str, tmplt), caseSensitivity));
}

ContainsMatcher IsEnum(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("enum " + str, caseSensitivity));
}

ContainsMatcher IsAbstractClass(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("abstract " + str, caseSensitivity));
}

ContainsMatcher IsBaseClass(std::string const &base, std::string const &sub,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(base + " <|-- " + sub, caseSensitivity));
}

ContainsMatcher IsInnerClass(std::string const &parent,
    std::string const &inner,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(parent + " +-- " + inner, caseSensitivity));
}

ContainsMatcher IsAssociation(std::string const &from, std::string const &to,
    std::string const &label,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} --> {} : {}", from, to, label), caseSensitivity));
}

ContainsMatcher IsComposition(std::string const &from, std::string const &to,
    std::string const &label,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} *-- {} : {}", from, to, label), caseSensitivity));
}

ContainsMatcher IsAggregation(std::string const &from, std::string const &to,
    std::string const &label,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} o-- {} : {}", from, to, label), caseSensitivity));
}

ContainsMatcher IsMethod(std::string const &name,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(name + "()", caseSensitivity));
}

ContainsMatcher IsField(std::string const &name,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(name, caseSensitivity));
}
}
}
}
