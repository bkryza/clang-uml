/**
 * tests/t00050/test_case.h
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

TEST_CASE("t00050", "[test-case][class]")
{
    auto [config, db] = load_config("t00050");

    auto diagram = config.diagrams["t00050_class"];

    REQUIRE(diagram->name == "t00050_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00050_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("utils::D")));
        REQUIRE_THAT(src, IsEnum(_A("E")));

        REQUIRE_THAT(src, HasNote(_A("A"), "left"));
        REQUIRE_THAT(src, HasNote(_A("A"), "right"));
        REQUIRE_THAT(src, HasNote(_A("B"), "top"));
        REQUIRE_THAT(src, HasNote(_A("C"), "top"));
        REQUIRE_THAT(src, HasNote(_A("utils::D"), "top"));
        REQUIRE_THAT(src, !HasNote(_A("E"), "bottom"));
        REQUIRE_THAT(src, !HasNote(_A("NoComment"), "top"));
        REQUIRE_THAT(src, HasNote(_A("F<T,V,int N>"), "top"));
        REQUIRE_THAT(src, HasNote(_A("G"), "top"));
        REQUIRE_THAT(src, HasNote(_A("G"), "bottom"));
        REQUIRE_THAT(src, HasNote(_A("G"), "right"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClass(j, "B"));
        REQUIRE(IsClass(j, "C"));
        REQUIRE(IsClass(j, "utils::D"));
        REQUIRE(IsEnum(j, "E"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::HasNote;
        using mermaid::IsEnum;

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("utils::D")));
        REQUIRE_THAT(src, IsEnum(_A("E")));

        REQUIRE_THAT(src, HasNote(_A("A"), "left"));
        REQUIRE_THAT(src, HasNote(_A("A"), "right"));
        REQUIRE_THAT(src, HasNote(_A("B"), "top"));
        REQUIRE_THAT(src, HasNote(_A("C"), "top"));
        REQUIRE_THAT(src, HasNote(_A("utils::D"), "top"));
        REQUIRE_THAT(src, !HasNote(_A("E"), "bottom"));
        REQUIRE_THAT(src, !HasNote(_A("NoComment"), "top"));
        REQUIRE_THAT(src, HasNote(_A("F<T,V,int N>"), "top"));
        REQUIRE_THAT(src, HasNote(_A("G"), "top"));
        REQUIRE_THAT(src, HasNote(_A("G"), "bottom"));
        REQUIRE_THAT(src, HasNote(_A("G"), "right"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}