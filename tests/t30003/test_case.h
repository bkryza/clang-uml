/**
 * tests/t30003/test_case.cc
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

TEST_CASE("t30003", "[test-case][package]")
{
    auto [config, db] = load_config("t30003");

    auto diagram = config.diagrams["t30003_package"];

    REQUIRE(diagram->name == "t30003_package");

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == "t30003_package");
    {
        auto src = generate_package_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsPackage("ns1"));
        REQUIRE_THAT(src, IsPackage("ns2"));
        REQUIRE_THAT(src, IsPackage("ns3"));
        REQUIRE_THAT(src, IsPackage("ns2_v1_0_0"));
        REQUIRE_THAT(src, IsPackage("ns2_v0_9_0"));

        REQUIRE_THAT(src, IsDeprecated(_A("ns2_v0_9_0")));
        REQUIRE_THAT(src, IsDeprecated(_A("ns3")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_package_json(diagram, *model);

        using namespace json;

        REQUIRE(IsPackage(j, "ns1"));
        REQUIRE(IsPackage(j, "ns1::ns2_v1_0_0"));
        REQUIRE(IsPackage(j, "ns1::ns2_v0_9_0"));
        REQUIRE(IsPackage(j, "ns3"));
        REQUIRE(IsPackage(j, "ns3::ns1"));
        REQUIRE(IsPackage(j, "ns3::ns1::ns2"));

        REQUIRE(IsDeprecated(j, "ns1::ns2_v0_9_0"));
        REQUIRE(IsDeprecated(j, "ns3"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_package_mermaid(diagram, *model);
        mermaid::AliasMatcher _A(src);
        using mermaid::IsPackage;

        REQUIRE_THAT(src, IsPackage(_A("ns1")));
        REQUIRE_THAT(src, IsPackage(_A("ns2")));
        REQUIRE_THAT(src, IsPackage(_A("ns3")));
        REQUIRE_THAT(src, IsPackage(_A("ns2_v1_0_0")));
        REQUIRE_THAT(src, IsPackage(_A("ns2_v0_9_0")));

        //        REQUIRE_THAT(src, IsDeprecated(_A("ns2_v0_9_0")));
        //        REQUIRE_THAT(src, IsDeprecated(_A("ns3")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
