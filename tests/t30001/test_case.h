/**
 * tests/t30001/test_case.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t30001", "[test-case][package]")
{
    auto [config, db] = load_config("t30001");

    auto diagram = config.diagrams["t30001_package"];

    REQUIRE(diagram->include.namespaces.size() == 1);
    REQUIRE_THAT(diagram->include.namespaces,
        VectorContains(std::string{"clanguml::t30001"}));

    REQUIRE(diagram->exclude.namespaces.size() == 1);
    REQUIRE_THAT(diagram->exclude.namespaces,
        VectorContains(std::string{"clanguml::t30001::detail"}));

    REQUIRE(diagram->should_include("clanguml::t30001::A"));
    REQUIRE(!diagram->should_include("clanguml::t30001::detail::C"));
    REQUIRE(!diagram->should_include("std::vector"));

    REQUIRE(diagram->name == "t30001_package");

    auto model = generate_package_diagram(db, diagram);

    REQUIRE(model.name() == "t30001_package");

    auto puml = generate_package_puml(diagram, model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, Contains("component [A]"));
    REQUIRE_THAT(puml, Contains("component [AA]"));
    REQUIRE_THAT(puml, Contains("component [AAA]"));

    REQUIRE_THAT(puml, Equals(R"(@startuml
' t30001 test package diagram
component [A] as C_0000000561 {
component [AA] as C_0000000562 {
component [AAA] as C_0000000563 {
}
component [BBB] as C_0000000564 {
}
}
component [BB] as C_0000000565 {
}
}

component [B] as C_0000000566 {
component [AA] as C_0000000567 {
component [AAA] as C_0000000568 {
}
component [BBB] as C_0000000569 {
}
}
component [BB] as C_0000000570 {
}
}

note right of C_0000000563: A AAA note...
@enduml
)"));

    save_puml(
        "./" + config.output_directory + "/" + diagram->name + ".puml", puml);
}
