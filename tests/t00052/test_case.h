/**
 * tests/t00052/test_case.h
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

TEST_CASE("t00052")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00052");

    auto diagram = config.diagrams["t00052_class"];

    REQUIRE(diagram->name == "t00052_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00052_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));

        REQUIRE(IsClassTemplate(src, "B<T>"));

        REQUIRE(IsMethod<Public>(src, "A", "a<T>", "T", "T p"));
        REQUIRE(IsMethod<Public>(src, "A", "aa<F,Q>", "void", "F && f, Q q"));
        REQUIRE(IsMethod<Public>(src, "B<T>", "b", "T", "T t"));
        REQUIRE(IsMethod<Public>(src, "B<T>", "bb<F>", "T", "F && f, T t"));
    });
    /*
    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));

        // Check if class templates exist
        REQUIRE_THAT(src, IsClassTemplate("B", "T"));

        // Check if all methods exist
        REQUIRE_THAT(src, (IsMethod<Public>("a<T>", "T", "T p")));
        REQUIRE_THAT(src, (IsMethod<Public>("aa<F,Q>", "void", "F && f, Q q")));
        REQUIRE_THAT(src, (IsMethod<Public>("b", "T", "T t")));
        REQUIRE_THAT(src, (IsMethod<Public>("bb<F>", "T", "F && f, T t")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClassTemplate(j, "B<T>"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsMethod;

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));

        // Check if class templates exist
        REQUIRE_THAT(src, IsClass(_A("B<T>")));

        // Check if all methods exist
        REQUIRE_THAT(src, (IsMethod<Public>("a<T>", "T", "T p")));
        REQUIRE_THAT(src, (IsMethod<Public>("aa<F,Q>", "void", "F && f, Q q")));
        REQUIRE_THAT(src, (IsMethod<Public>("b", "T", "T t")));
        REQUIRE_THAT(src, (IsMethod<Public>("bb<F>", "T", "F && f, T t")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
     */
}