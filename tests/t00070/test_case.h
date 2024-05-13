/**
 * tests/t00070/test_case.h
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

TEST_CASE("t00070")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00070");

    auto diagram = config.diagrams["t00070_class"];

    REQUIRE(diagram->name == "t00070_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00070_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(!IsClass(src, "C"));

        REQUIRE(IsClassTemplate(src, "BB<T>"));
        REQUIRE(!IsClassTemplate(src, "CC<T>"));

        REQUIRE(IsEnum(src, "BBB"));
        REQUIRE(!IsClass(src, "BBBB"));
        REQUIRE(!IsEnum(src, "CCC"));
    });

    /*
        {
            auto src = generate_class_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));


            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_class_json(diagram, *model);

            using namespace json;

            REQUIRE(IsClass(j, "A"));
            REQUIRE(IsClass(j, "B"));
            REQUIRE(!IsClass(j, "C"));

            REQUIRE(InPublicModule(j, "A", "t00070"));
            REQUIRE(InPublicModule(j, "B", "t00070.lib1"));

            REQUIRE(!IsClass(j, "BBBB"));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_class_mermaid(diagram, *model);

            mermaid::AliasMatcher _A(src);
            using mermaid::IsClass;
            using mermaid::IsEnum;

            REQUIRE_THAT(src, IsClass(_A("A")));
            REQUIRE_THAT(src, IsClass(_A("B")));
            REQUIRE_THAT(src, !IsClass(_A("C")));

            REQUIRE_THAT(src, IsClass(_A("BB<T>")));
            REQUIRE_THAT(src, !IsClass(_A("CC<T>")));

            REQUIRE_THAT(src, IsEnum(_A("BBB")));
            REQUIRE_THAT(src, !IsClass(_A("BBBB")));
            REQUIRE_THAT(src, !IsEnum(_A("CCC")));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }*/
}