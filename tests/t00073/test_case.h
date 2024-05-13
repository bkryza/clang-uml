/**
 * tests/t00073/test_case.h
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

TEST_CASE("t00073")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00073");

    auto diagram = config.diagrams["t00073_class"];

    REQUIRE(diagram->name == "t00073_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00073_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, "AHandler"));
        REQUIRE(IsClass(src, "BHandler"));
        REQUIRE(IsClassTemplate(src, "Overload<Bases...>"));
        REQUIRE(IsClassTemplate(src, "Overload<AHandler,BHandler>"));

        REQUIRE(IsDependency(src, "AHandler", "A"));
        REQUIRE(IsDependency(src, "BHandler", "B"));
        REQUIRE(IsInstantiation(
            src, "Overload<Bases...>", "Overload<AHandler,BHandler>"));
        REQUIRE(IsAggregation<Public>(
            src, "R", "Overload<AHandler,BHandler>", "dispatch"));
    });
    /*
        {
            auto src = generate_class_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            REQUIRE_THAT(src, IsClass(_A("A")));
            REQUIRE_THAT(src, IsClass(_A("B")));
            REQUIRE_THAT(src, IsClass(_A("AHandler")));
            REQUIRE_THAT(src, IsClass(_A("BHandler")));
            REQUIRE_THAT(src, IsClassTemplate("Overload", "Bases..."));
            REQUIRE_THAT(src, IsClassTemplate("Overload", "AHandler,BHandler"));

            REQUIRE_THAT(src, IsDependency(_A("AHandler"), _A("A")));
            REQUIRE_THAT(src, IsDependency(_A("BHandler"), _A("B")));
            REQUIRE_THAT(src,
                IsInstantiation(
                    _A("Overload<Bases...>"),
       _A("Overload<AHandler,BHandler>"))); REQUIRE_THAT(src, IsAggregation(
                    _A("R"), _A("Overload<AHandler,BHandler>"), "+dispatch"));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_class_json(diagram, *model);

            using namespace json;

            REQUIRE(IsClassTemplate(j, "Overload<Bases...>"));
            REQUIRE(IsClass(j, "A"));
            REQUIRE(IsClass(j, "B"));
            REQUIRE(IsClass(j, "AHandler"));
            REQUIRE(IsClass(j, "BHandler"));
            REQUIRE(IsClass(j, "Overload<AHandler,BHandler>"));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_class_mermaid(diagram, *model);

            mermaid::AliasMatcher _A(src);
            using mermaid::IsClass;

            REQUIRE_THAT(src, IsClass(_A("A")));
            REQUIRE_THAT(src, IsClass(_A("B")));
            REQUIRE_THAT(src, IsClass(_A("AHandler")));
            REQUIRE_THAT(src, IsClass(_A("BHandler")));
            REQUIRE_THAT(src, IsClass(_A("Overload<Bases...>")));
            REQUIRE_THAT(src, IsClass(_A("Overload<AHandler,BHandler>")));

            REQUIRE_THAT(src, IsDependency(_A("AHandler"), _A("A")));
            REQUIRE_THAT(src, IsDependency(_A("BHandler"), _A("B")));
            REQUIRE_THAT(src,
                IsInstantiation(
                    _A("Overload<Bases...>"),
       _A("Overload<AHandler,BHandler>"))); REQUIRE_THAT(src, IsAggregation(
                    _A("R"), _A("Overload<AHandler,BHandler>"), "+dispatch"));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }*/
}