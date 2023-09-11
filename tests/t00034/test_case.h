/**
 * tests/t00034/test_case.cc
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

TEST_CASE("t00034", "[test-case][class]")
{
    auto [config, db] = load_config("t00034");

    auto diagram = config.diagrams["t00034_class"];

    REQUIRE(diagram->name == "t00034_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00034_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsClassTemplate("lift_void", "T"));
        REQUIRE_THAT(src, IsClassTemplate("drop_void", "T"));
        REQUIRE_THAT(src, IsClass(_A("Void")));
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("R")));

        REQUIRE_THAT(
            src, IsInstantiation(_A("lift_void<T>"), _A("lift_void<void>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("drop_void<T>"), _A("drop_void<Void>")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClassTemplate(j, "lift_void<T>"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);

        REQUIRE_THAT(src, IsClass(_A("lift_void<T>")));
        REQUIRE_THAT(src, IsClass(_A("drop_void<T>")));
        REQUIRE_THAT(src, IsClass(_A("Void")));
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("R")));

        REQUIRE_THAT(
            src, IsInstantiation(_A("lift_void<T>"), _A("lift_void<void>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("drop_void<T>"), _A("drop_void<Void>")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}