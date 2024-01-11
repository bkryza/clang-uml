/**
 * tests/t00068/test_case.h
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

TEST_CASE("t00068_r0", "[test-case][class][t00068]")
{
    auto [config, db] = load_config("t00068");

    auto diagram = config.diagrams["t00068_r0_class"];

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00068_r0_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, !IsClass(_A("A")));
        REQUIRE_THAT(src, !IsClass(_A("AA")));
        REQUIRE_THAT(src, IsClass(_A("AAA")));

        REQUIRE_THAT(src, !IsClass(_A("B")));
        REQUIRE_THAT(src, !IsClass(_A("BB")));

        REQUIRE_THAT(src, !IsClass(_A("R")));
        REQUIRE_THAT(src, !IsClass(_A("RR")));

        REQUIRE_THAT(src, !IsEnum(_A("AKind")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsClass;
        using mermaid::IsEnum;

        REQUIRE_THAT(src, !IsClass(_A("A")));
        REQUIRE_THAT(src, !IsClass(_A("AA")));
        REQUIRE_THAT(src, IsClass(_A("AAA")));

        REQUIRE_THAT(src, !IsClass(_A("B")));
        REQUIRE_THAT(src, !IsClass(_A("BB")));

        REQUIRE_THAT(src, !IsClass(_A("R")));
        REQUIRE_THAT(src, !IsClass(_A("RR")));

        REQUIRE_THAT(src, !IsEnum(_A("AKind")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}

TEST_CASE("t00068_r1", "[test-case][class][t00068]")
{
    auto [config, db] = load_config("t00068");

    auto diagram = config.diagrams["t00068_r1_class"];

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00068_r1_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, !IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("AA")));
        REQUIRE_THAT(src, IsClass(_A("AAA")));

        REQUIRE_THAT(src, !IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("BB")));

        REQUIRE_THAT(src, IsClass(_A("R")));
        REQUIRE_THAT(src, !IsClass(_A("RR")));

        REQUIRE_THAT(src, IsEnum(_A("AKind")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsClass;

        REQUIRE_THAT(src, !IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("AA")));
        REQUIRE_THAT(src, IsClass(_A("AAA")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}

TEST_CASE("t00068_r2", "[test-case][class][t00068]")
{
    auto [config, db] = load_config("t00068");

    auto diagram = config.diagrams["t00068_r2_class"];

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00068_r2_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("AA")));
        REQUIRE_THAT(src, IsClass(_A("AAA")));

        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("BB")));

        REQUIRE_THAT(src, IsClass(_A("R")));
        REQUIRE_THAT(src, IsClass(_A("RR")));

        REQUIRE_THAT(src, IsEnum(_A("AKind")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsClass;
        using mermaid::IsEnum;

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("AA")));
        REQUIRE_THAT(src, IsClass(_A("AAA")));

        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("BB")));

        REQUIRE_THAT(src, IsClass(_A("R")));
        REQUIRE_THAT(src, IsClass(_A("RR")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}