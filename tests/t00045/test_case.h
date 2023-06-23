/**
 * tests/t00045/test_case.cc
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

TEST_CASE("t00045", "[test-case][class]")
{
    auto [config, db] = load_config("t00045");

    auto diagram = config.diagrams["t00045_class"];

    REQUIRE(diagram->name == "t00045_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00045_class");

    {
        auto puml = generate_class_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));
        REQUIRE_THAT(puml, IsClass(_A("A")));
        REQUIRE_THAT(puml, IsClass(_A("ns1::A")));
        REQUIRE_THAT(puml, IsClass(_A("ns1::ns2::A")));
        REQUIRE_THAT(puml, IsClass(_A("ns1::ns2::B")));
        REQUIRE_THAT(puml, IsClass(_A("ns1::ns2::C")));
        REQUIRE_THAT(puml, IsClass(_A("ns1::ns2::D")));
        REQUIRE_THAT(puml, IsClass(_A("ns1::ns2::E")));
        REQUIRE_THAT(puml, IsClass(_A("ns1::ns2::R")));

        REQUIRE_THAT(puml, IsBaseClass(_A("ns1::ns2::A"), _A("ns1::ns2::B")));
        REQUIRE_THAT(puml, IsBaseClass(_A("ns1::A"), _A("ns1::ns2::C")));
        REQUIRE_THAT(puml, IsBaseClass(_A("ns1::ns2::A"), _A("ns1::ns2::D")));
        REQUIRE_THAT(puml, IsBaseClass(_A("A"), _A("ns1::ns2::E")));

        REQUIRE_THAT(
            puml, IsAssociation(_A("ns1::ns2::R"), _A("ns1::ns2::A"), "+a"));
        REQUIRE_THAT(
            puml, IsAssociation(_A("ns1::ns2::R"), _A("ns1::A"), "+ns1_a"));
        REQUIRE_THAT(puml,
            IsAssociation(_A("ns1::ns2::R"), _A("ns1::ns2::A"), "+ns1_ns2_a"));
        REQUIRE_THAT(
            puml, IsAssociation(_A("ns1::ns2::R"), _A("A"), "+root_a"));

        REQUIRE_THAT(puml, IsDependency(_A("ns1::ns2::R"), _A("AA")));

        REQUIRE_THAT(puml, IsFriend<Public>(_A("ns1::ns2::R"), _A("AAA")));
        REQUIRE_THAT(
            puml, !IsFriend<Public>(_A("ns1::ns2::R"), _A("ns1::ns2::AAA")));
        // TODO:
        // REQUIRE_THAT(puml, IsFriend<Public>(_A("ns1::ns2::R"),
        // _A("AAAA<T>")));

        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClass(j, "ns1::A"));
        REQUIRE(IsClass(j, "ns1::ns2::A"));
        REQUIRE(IsClass(j, "ns1::ns2::B"));
        REQUIRE(IsClass(j, "ns1::ns2::C"));
        REQUIRE(IsClass(j, "ns1::ns2::D"));
        REQUIRE(IsClass(j, "ns1::ns2::E"));
        REQUIRE(IsClass(j, "ns1::ns2::R"));

        REQUIRE(IsBaseClass(j, "ns1::ns2::A", "ns1::ns2::B"));

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}
