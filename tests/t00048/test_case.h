/**
 * tests/t00048/test_case.h
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

TEST_CASE("t00048", "[test-case][class]")
{
    auto [config, db] = load_config("t00048");

    auto diagram = config.diagrams["t00048_class"];

    REQUIRE(diagram->name == "t00048_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00048_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsAbstractClass(_A("Base")));
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));

        // Check if class templates exist
        REQUIRE_THAT(src, IsAbstractClassTemplate("BaseTemplate", "T"));
        REQUIRE_THAT(src, IsClassTemplate("ATemplate", "T"));
        REQUIRE_THAT(src, IsClassTemplate("BTemplate", "T"));

        // Check if all inheritance relationships exist
        REQUIRE_THAT(src, IsBaseClass(_A("Base"), _A("A")));
        REQUIRE_THAT(src, IsBaseClass(_A("Base"), _A("B")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClass(j, "B"));
        REQUIRE(IsClass(j, "ATemplate<T>"));
        REQUIRE(IsClass(j, "BTemplate<T>"));
        REQUIRE(IsBaseClass(j, "Base", "A"));
        REQUIRE(IsBaseClass(j, "Base", "B"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsAbstractClass;

        // Check if all classes exist
        REQUIRE_THAT(src, IsAbstractClass(_A("Base")));
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));

        // Check if class templates exist
        REQUIRE_THAT(src, IsAbstractClass(_A("BaseTemplate<T>")));
        REQUIRE_THAT(src, IsClass(_A("ATemplate<T>")));
        REQUIRE_THAT(src, IsClass(_A("BTemplate<T>")));

        // Check if all inheritance relationships exist
        REQUIRE_THAT(src, IsBaseClass(_A("Base"), _A("A")));
        REQUIRE_THAT(src, IsBaseClass(_A("Base"), _A("B")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}