/**
 * @file tests/test_decorator_parser.cc
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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "decorators/decorators.h"
#include "util/util.h"

#include "doctest/doctest.h"

TEST_CASE("Test decorator parser on regular comment")
{
    std::string comment = R"(
    \brief This is a comment.

    This is a longer comment.

    \param a int an int
    \param b float a float

    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment);

    CHECK(decorators.empty());
    CHECK(clanguml::util::trim(comment) == stripped);
}

TEST_CASE("Test decorator parser on note")
{
    std::string comment = R"(
    \brief This is a comment.

    This is a longer comment.

    \

    \param a int an int
    \param b float a float

    \uml{note[left] This is a comment}
    @uml{note[ top  ]

            This is a comment    }
    \uml{note This is a comment}
    \uml{note[] This is a comment}
    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment);

    CHECK(decorators.size() == 4);

    auto n1 = std::dynamic_pointer_cast<note>(decorators.at(0));
    auto n2 = std::dynamic_pointer_cast<note>(decorators.at(1));
    auto n3 = std::dynamic_pointer_cast<note>(decorators.at(2));
    auto n4 = std::dynamic_pointer_cast<note>(decorators.at(3));

    CHECK(n1);
    CHECK(n1->position == "left");
    CHECK(n1->text == "This is a comment");

    CHECK(n2);
    CHECK(n2->position == "top");
    CHECK(n2->text == "This is a comment");

    CHECK(n3);
    CHECK(n3->position == "left");
    CHECK(n3->text == "This is a comment");

    CHECK(n4);
    CHECK(n4->position == "left");
    CHECK(n4->text == "This is a comment");

    CHECK(stripped == R"(\brief This is a comment.

    This is a longer comment.

    \

    \param a int an int
    \param b float a float)");
}

TEST_CASE("Test decorator parser on note with custom tag")
{
    std::string comment = R"(
    \brief This is a comment.

    This is a longer comment.

    \

    \param a int an int
    \param b float a float

    \clanguml{note[left] This is a comment}
    @clanguml{note[ top  ]

            This is a comment    }
    \clanguml{note This is a comment}
    \clanguml{note[] This is a comment}
    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment, "clanguml");

    CHECK(decorators.size() == 4);

    auto n1 = std::dynamic_pointer_cast<note>(decorators.at(0));
    auto n2 = std::dynamic_pointer_cast<note>(decorators.at(1));
    auto n3 = std::dynamic_pointer_cast<note>(decorators.at(2));
    auto n4 = std::dynamic_pointer_cast<note>(decorators.at(3));

    CHECK(n1);
    CHECK(n1->position == "left");
    CHECK(n1->text == "This is a comment");

    CHECK(n2);
    CHECK(n2->position == "top");
    CHECK(n2->text == "This is a comment");

    CHECK(n3);
    CHECK(n3->position == "left");
    CHECK(n3->text == "This is a comment");

    CHECK(n4);
    CHECK(n4->position == "left");
    CHECK(n4->text == "This is a comment");

    CHECK(stripped == R"(\brief This is a comment.

    This is a longer comment.

    \

    \param a int an int
    \param b float a float)");
}

TEST_CASE("Test decorator parser on style")
{
    std::string comment = R"(
    \uml{style[#green,dashed,thickness=4]}
    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<style>(decorators.at(0));

    CHECK(n1);
    CHECK(n1->spec == "#green,dashed,thickness=4");
    CHECK(stripped.empty());
}

TEST_CASE("Test decorator parser on aggregation")
{
    std::string comment = R"(
    \uml{aggregation[0..1:0..*]}
    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<aggregation>(decorators.at(0));

    CHECK(n1);
    CHECK(n1->multiplicity == "0..1:0..*");
    CHECK(stripped.empty());
}

TEST_CASE("Test decorator parser on skip")
{
    std::string comment = R"(
    \uml{skip}
    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<skip>(decorators.at(0));

    CHECK(n1);
    CHECK(stripped.empty());
}

TEST_CASE("Test decorator parser on skiprelationship")
{
    std::string comment = R"(
    \uml{skiprelationship}
    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<skip_relationship>(decorators.at(0));

    CHECK(n1);
    CHECK(stripped.empty());
}

TEST_CASE("Test decorator parser on diagram scope")
{
    std::string comment = R"(
    \uml{note:diagram1,  diagram2,
    diagram3[left] Note only for diagrams 1, 2 and 3.}
    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment);

    CHECK(decorators.size() == 1);

    auto n1 = std::dynamic_pointer_cast<note>(decorators.at(0));

    CHECK(n1);
    CHECK(n1->diagrams.size() == 3);
    CHECK(n1->diagrams[0] == "diagram1");
    CHECK(n1->diagrams[1] == "diagram2");
    CHECK(n1->diagrams[2] == "diagram3");

    CHECK(n1->position == "left");
    CHECK(n1->text == "Note only for diagrams 1, 2 and 3.");

    CHECK(n1->applies_to_diagram("diagram2"));
    CHECK(!n1->applies_to_diagram("diagram4"));
    CHECK(stripped.empty());
}

TEST_CASE("Test invalid comment - unterminated curly brace")
{
    std::string comment = R"(
    Test test test
    \uml{call clanguml::test:aa()
    )";

    using namespace clanguml::decorators;

    auto [decorators, stripped] = parse(comment);

    CHECK(decorators.size() == 0);

    CHECK(stripped.empty());
}
