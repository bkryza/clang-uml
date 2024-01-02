/**
 * tests/t00060/test_case.h
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

TEST_CASE("t00060", "[test-case][class]")
{
    auto [config, db] = load_config("t00060");

    auto diagram = config.diagrams["t00060_class"];

    REQUIRE(diagram->name == "t00060_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00060_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, !IsClass(_A("E")));
        REQUIRE_THAT(src, !IsClass(_A("F")));

        // Check if class templates exist
        REQUIRE_THAT(src, IsClassTemplate("G", "T"));
        REQUIRE_THAT(src, IsClassTemplate("H", "T,P"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClass(j, "B"));
        REQUIRE(IsClass(j, "C"));
        REQUIRE(IsClass(j, "D"));
        REQUIRE(!IsClass(j, "E"));
        REQUIRE(!IsClass(j, "F"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, !IsClass(_A("E")));
        REQUIRE_THAT(src, !IsClass(_A("F")));

        // Check if class templates exist
        REQUIRE_THAT(src, IsClass(_A("G<T>")));
        REQUIRE_THAT(src, IsClass(_A("H<T,P>")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}