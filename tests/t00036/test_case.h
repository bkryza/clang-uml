/**
 * tests/t00036/test_case.cc
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

TEST_CASE("t00036", "[test-case][class]")
{
    auto [config, db] = load_config("t00036");

    auto diagram = config.diagrams["t00036_class"];

    REQUIRE(diagram->name == "t00036_class");
    REQUIRE(diagram->generate_packages() == true);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00036_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsClassTemplate("A", "T"));
        REQUIRE_THAT(src, IsClassTemplate("A", "int"));
        REQUIRE_THAT(src, IsEnum(_A("E")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, !IsClass(_A("DImpl")));
        REQUIRE_THAT(src, IsPackage("ns111"));
        REQUIRE_THAT(src, IsPackage("ns22"));
        REQUIRE_THAT(src, !IsPackage("ns3"));
        REQUIRE_THAT(src, !IsPackage("ns33"));

        REQUIRE_THAT(src, IsAggregation(_A("B"), _A("A<int>"), "+a_int"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;
        using namespace std::string_literals;

        REQUIRE(IsClass(j, "ns1::ns11::A<T>"));
        REQUIRE(IsClass(j, "ns1::ns11::A<int>"));
        REQUIRE(IsClass(j, "ns1::ns11::ns111::B"));
        REQUIRE(IsClass(j, "ns2::ns22::C"));
        REQUIRE(IsEnum(j, "ns1::E"));
        REQUIRE(IsNamespacePackage(j, "ns1"s));
        REQUIRE(IsNamespacePackage(j, "ns1"s, "ns11"s));
        REQUIRE(IsNamespacePackage(j, "ns1"s, "ns11"s, "ns111"s));
        REQUIRE(IsNamespacePackage(j, "ns2"s));
        REQUIRE(IsNamespacePackage(j, "ns2"s, "ns22"s));
        REQUIRE(IsNamespacePackage(j, "ns3"s));
        REQUIRE(IsNamespacePackage(j, "ns3"s, "ns33"s));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsEnum;

        REQUIRE_THAT(src, IsClass(_A("ns1::ns11::A<T>")));
        REQUIRE_THAT(src, IsClass(_A("ns1::ns11::A<int>")));
        REQUIRE_THAT(src, IsEnum(_A("ns1::E")));
        REQUIRE_THAT(src, IsClass(_A("ns1::ns11::ns111::B")));
        REQUIRE_THAT(src, IsClass(_A("ns2::ns22::C")));
        REQUIRE_THAT(src, !IsClass(_A("DImpl")));

        REQUIRE_THAT(src,
            IsAggregation(
                _A("ns1::ns11::ns111::B"), _A("ns1::ns11::A<int>"), "+a_int"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}