/**
 * tests/t00041/test_case.cc
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

TEST_CASE("t00041", "[test-case][class]")
{
    auto [config, db] = load_config("t00041");

    auto diagram = config.diagrams["t00041_class"];

    REQUIRE(diagram->name == "t00041_class");
    REQUIRE(diagram->generate_packages() == false);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00041_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, !IsClass(_A("A")));
        REQUIRE_THAT(src, !IsClass(_A("AA")));
        REQUIRE_THAT(src, !IsClass(_A("AAA")));

        REQUIRE_THAT(src, !IsClass(_A("B")));

        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, IsClass(_A("E")));
        REQUIRE_THAT(src, IsClass(_A("F")));
        REQUIRE_THAT(src, IsClass(_A("R")));
        REQUIRE_THAT(src, IsClass(_A("RR")));
        REQUIRE_THAT(src, IsClass(_A("RRR")));
        REQUIRE_THAT(src, !IsClass(_A("detail::G")));
        REQUIRE_THAT(src, !IsClass(_A("H")));

        REQUIRE_THAT(src, IsBaseClass(_A("R"), _A("RR")));
        REQUIRE_THAT(src, IsBaseClass(_A("RR"), _A("RRR")));

        REQUIRE_THAT(src, IsAssociation(_A("D"), _A("RR"), "+rr"));
        REQUIRE_THAT(src, IsAssociation(_A("RR"), _A("E"), "+e"));
        REQUIRE_THAT(src, IsAssociation(_A("RR"), _A("F"), "+f"));
        REQUIRE_THAT(src, !IsDependency(_A("RR"), _A("H")));

        REQUIRE_THAT(src, IsClass(_A("ns1::N")));
        REQUIRE_THAT(src, IsClass(_A("ns1::NN")));
        REQUIRE_THAT(src, IsClass(_A("ns1::NM")));
        REQUIRE_THAT(src, IsBaseClass(_A("ns1::N"), _A("ns1::NN")));
        REQUIRE_THAT(src, IsBaseClass(_A("ns1::N"), _A("ns1::NM")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(!IsClass(j, "A"));
        REQUIRE(!IsClass(j, "AA"));
        REQUIRE(!IsClass(j, "AAA"));
        REQUIRE(IsClass(j, "D"));
        REQUIRE(IsClass(j, "E"));
        REQUIRE(IsClass(j, "F"));
        REQUIRE(IsClass(j, "R"));

        REQUIRE(IsAssociation(j, "D", "RR", "rr"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);

        REQUIRE_THAT(src, !IsClass(_A("A")));
        REQUIRE_THAT(src, !IsClass(_A("AA")));
        REQUIRE_THAT(src, !IsClass(_A("AAA")));

        REQUIRE_THAT(src, !IsClass(_A("B")));

        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, IsClass(_A("E")));
        REQUIRE_THAT(src, IsClass(_A("F")));
        REQUIRE_THAT(src, IsClass(_A("R")));
        REQUIRE_THAT(src, IsClass(_A("RR")));
        REQUIRE_THAT(src, IsClass(_A("RRR")));
        REQUIRE_THAT(src, !IsClass(_A("detail::G")));
        REQUIRE_THAT(src, !IsClass(_A("H")));

        REQUIRE_THAT(src, IsBaseClass(_A("R"), _A("RR")));
        REQUIRE_THAT(src, IsBaseClass(_A("RR"), _A("RRR")));

        REQUIRE_THAT(src, IsAssociation(_A("D"), _A("RR"), "+rr"));
        REQUIRE_THAT(src, IsAssociation(_A("RR"), _A("E"), "+e"));
        REQUIRE_THAT(src, IsAssociation(_A("RR"), _A("F"), "+f"));
        REQUIRE_THAT(src, !IsDependency(_A("RR"), _A("H")));

        REQUIRE_THAT(src, IsClass(_A("ns1::N")));
        REQUIRE_THAT(src, IsClass(_A("ns1::NN")));
        REQUIRE_THAT(src, IsClass(_A("ns1::NM")));
        REQUIRE_THAT(src, IsBaseClass(_A("ns1::N"), _A("ns1::NN")));
        REQUIRE_THAT(src, IsBaseClass(_A("ns1::N"), _A("ns1::NM")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}