/**
 * tests/t00069/test_case.h
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

TEST_CASE("t00069")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00069");

    auto diagram = config.diagrams["t00069_class"];

    REQUIRE(diagram->name == "t00069_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00069_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));

        REQUIRE(IsClassTemplate(src, "generator", "T"));

        REQUIRE(
            IsInnerClass(src,("generator<T>", "generator::promise_type"));

        REQUIRE(
            IsMethod<Public, Coroutine>(src,"iota", "generator<unsigned long>"));
        REQUIRE(
            IsMethod<Public, Coroutine>(src,"seed", "generator<unsigned long>"));

        REQUIRE(
            IsDependency(src,"A", "generator<unsigned long>"));
        REQUIRE(
            IsInstantiation(
                src,"generator<T>", "generator<unsigned long>"));
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
            REQUIRE_THAT(src, IsClassTemplate("generator", "T"));

            // Check if all inner classes exist
            REQUIRE_THAT(src,
                IsInnerClass(_A("generator<T>"),
       _A("generator::promise_type")));

            // Check if all methods exist
            REQUIRE_THAT(src,
                (IsMethod<Public, Coroutine>("iota", "generator<unsigned
       long>"))); REQUIRE_THAT(src, (IsMethod<Public, Coroutine>("seed",
       "generator<unsigned long>")));

            // Check if all relationships exist
            REQUIRE_THAT(
                src, IsDependency(_A("A"), _A("generator<unsigned long>")));
            REQUIRE_THAT(src,
                IsInstantiation(
                    _A("generator<T>"), _A("generator<unsigned long>")));

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
            using mermaid::IsMethod;

            REQUIRE_THAT(src, IsClass(_A("A")));
            REQUIRE_THAT(src,
                (IsMethod<Public, Coroutine>("iota", "generator<unsigned
       long>"))); REQUIRE_THAT(src, (IsMethod<Public, Coroutine>("seed",
       "generator<unsigned long>")));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }*/
}