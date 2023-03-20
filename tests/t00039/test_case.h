/**
 * tests/t00039/test_case.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t00039", "[test-case][class]")
{
    auto [config, db] = load_config("t00039");

    auto diagram = config.diagrams["t00039_class"];

    REQUIRE(diagram->name == "t00039_class");
    REQUIRE(diagram->generate_packages() == false);
    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00039_class");

    {
        auto puml = generate_class_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        REQUIRE_THAT(puml, IsClass(_A("A")));
        REQUIRE_THAT(puml, IsClass(_A("AA")));
        REQUIRE_THAT(puml, IsClass(_A("AAA")));
        REQUIRE_THAT(puml, IsClass(_A("ns2::AAAA")));
        REQUIRE_THAT(puml, IsBaseClass(_A("A"), _A("AA")));
        REQUIRE_THAT(puml, IsBaseClass(_A("AA"), _A("AAA")));
        REQUIRE_THAT(puml, IsBaseClass(_A("AAA"), _A("ns2::AAAA")));
        REQUIRE_THAT(puml, !IsClass(_A("detail::AA")));

        REQUIRE_THAT(puml, !IsClass(_A("B")));
        REQUIRE_THAT(puml, !IsClass(_A("ns1::BB")));

        REQUIRE_THAT(puml, IsClass(_A("C")));
        REQUIRE_THAT(puml, IsClass(_A("D")));
        REQUIRE_THAT(puml, IsClass(_A("E")));
        REQUIRE_THAT(puml, IsBaseClass(_A("C"), _A("CD")));
        REQUIRE_THAT(puml, IsBaseClass(_A("D"), _A("CD")));
        REQUIRE_THAT(puml, IsBaseClass(_A("D"), _A("DE")));
        REQUIRE_THAT(puml, IsBaseClass(_A("E"), _A("DE")));
        REQUIRE_THAT(puml, IsBaseClass(_A("C"), _A("CDE")));
        REQUIRE_THAT(puml, IsBaseClass(_A("D"), _A("CDE")));
        REQUIRE_THAT(puml, IsBaseClass(_A("E"), _A("CDE")));

        REQUIRE_THAT(puml, IsClassTemplate("ns3::F", "T"));
        REQUIRE_THAT(puml, IsClassTemplate("ns3::FF", "T,M"));
        REQUIRE_THAT(puml, IsClassTemplate("ns3::FE", "T,M"));
        REQUIRE_THAT(puml, IsClassTemplate("ns3::FFF", "T,M,N"));

        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}
