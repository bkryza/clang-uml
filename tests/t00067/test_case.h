/**
 * tests/t00067/test_case.h
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

TEST_CASE("t00067")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00067");

    auto diagram = config.diagrams["t00067_class"];

    REQUIRE(diagram->name == "t00067_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00067_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(!(IsMethod<Public, Default>(src, "A")));
        REQUIRE(!(IsMethod<Public, Default>(src, "A", "void", "A &&")));
        REQUIRE(!(IsMethod<Public, Deleted>(src, "A", "void", "const A &")));

        REQUIRE(!(IsMethod<Public, Default>(src, "~A")));

        REQUIRE(!(IsMethod<Public, Default>(src, "~A")));
    });
    /*
    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, !(IsMethod<Public, Default>("A")));
        REQUIRE_THAT(src, !(IsMethod<Public, Default>("A", "void", "A &&")));
        REQUIRE_THAT(
            src, !(IsMethod<Public, Deleted>("A", "void", "const A &")));

        REQUIRE_THAT(src, !(IsMethod<Public, Default>("~A")));

        REQUIRE_THAT(src, !(IsMethod<Public, Default>("~A")));

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
        using mermaid::IsMethod;

        REQUIRE_THAT(src, !(IsMethod<Public, Default>("A")));
        REQUIRE_THAT(src, !(IsMethod<Public, Default>("A", "void", "A &&")));
        REQUIRE_THAT(
            src, !(IsMethod<Public, Deleted>("A", "void", "const A &")));

        REQUIRE_THAT(src, !(IsMethod<Public, Default>("~A")));

        REQUIRE_THAT(src, !(IsMethod<Public, Default>("~A")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}