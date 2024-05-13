/**
 * tests/t00041/test_case.h
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

TEST_CASE("t00041")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00041");

    auto diagram = config.diagrams["t00041_class"];

    REQUIRE(diagram->name == "t00041_class");
    REQUIRE(diagram->generate_packages() == false);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00041_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(!IsClass(src, "A"));
        REQUIRE(!IsClass(src, "AA"));
        REQUIRE(!IsClass(src, "AAA"));

        REQUIRE(!IsClass(src, "B"));

        REQUIRE(IsClass(src, "D"));
        REQUIRE(IsClass(src, "E"));
        REQUIRE(IsClass(src, "F"));
        REQUIRE(IsClass(src, "R"));
        REQUIRE(IsClass(src, "RR"));
        REQUIRE(IsClass(src, "RRR"));
        REQUIRE(!IsClass(src, "detail::G"));
        REQUIRE(!IsClass(src, "H"));

        REQUIRE(IsBaseClass(src, "R", "RR"));
        REQUIRE(IsBaseClass(src, "RR", "RRR"));

        REQUIRE(IsAssociation<Public>(src, "D", "RR", "rr"));
        REQUIRE(IsAssociation<Public>(src, "RR", "E", "e"));
        REQUIRE(IsAssociation<Public>(src, "RR", "F", "f"));
        REQUIRE(!IsDependency(src, "RR", "H"));

        REQUIRE(IsClass(src, {"ns1", "N"}));
        REQUIRE(IsClass(src, {"ns1", "NN"}));
        REQUIRE(IsClass(src, {"ns1", "NM"}));
        REQUIRE(IsBaseClass(src, "ns1::N", "ns1::NN"));
        REQUIRE(IsBaseClass(src, "ns1::N", "ns1::NM"));
    });
}