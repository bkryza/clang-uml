/**
 * tests/t00054/test_case.h
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

TEST_CASE("t00054")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00054");

    auto diagram = config.diagrams["t00054_class"];

    REQUIRE(diagram->name == "t00054_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00054_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "a"));
        REQUIRE(IsClass(src, "b"));
        REQUIRE(IsClass(src, {"detail", "c"}));
        REQUIRE(IsClass(src, {"detail", "d"}));
        REQUIRE(IsClass(src, {"detail", "e"}));
        REQUIRE(IsClass(src, "f"));
        REQUIRE(IsClass(src, "g"));

        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, {"detail2", "C"}));
        REQUIRE(IsClass(src, {"detail2::detail3", "D"}));
        REQUIRE(IsClass(src, {"detail2::detail3", "E"}));
        REQUIRE(IsClass(src, {"detail2", "F"}));
        REQUIRE(IsClass(src, "G"));

        REQUIRE(IsEnum(src, {"detail4", "i"}));
        REQUIRE(IsEnum(src, {"detail4", "h"}));
        REQUIRE(IsEnum(src, {"detail4", "j"}));
    });
    /*
    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("a")));
        REQUIRE_THAT(src, IsClass(_A("b")));
        REQUIRE_THAT(src, IsClass(_A("c")));
        REQUIRE_THAT(src, IsClass(_A("d")));
        REQUIRE_THAT(src, IsClass(_A("e")));
        REQUIRE_THAT(src, IsClass(_A("f")));
        REQUIRE_THAT(src, IsClass(_A("g")));

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, IsClass(_A("E")));
        REQUIRE_THAT(src, IsClass(_A("F")));
        REQUIRE_THAT(src, IsClass(_A("G")));

        REQUIRE_THAT(src, IsEnum(_A("i")));
        REQUIRE_THAT(src, IsEnum(_A("h")));
        REQUIRE_THAT(src, IsEnum(_A("j")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "a"));
        REQUIRE(IsClass(j, "b"));
        REQUIRE(IsClass(j, "detail::c"));
        REQUIRE(IsClass(j, "detail::d"));
        REQUIRE(IsClass(j, "detail::e"));
        REQUIRE(IsClass(j, "f"));
        REQUIRE(IsClass(j, "g"));

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClass(j, "B"));
        REQUIRE(IsClass(j, "detail2::C"));
        REQUIRE(IsClass(j, "detail2::detail3::D"));
        REQUIRE(IsClass(j, "detail2::detail3::E"));
        REQUIRE(IsClass(j, "detail2::F"));
        REQUIRE(IsClass(j, "G"));

        REQUIRE(IsEnum(j, "detail4::i"));
        REQUIRE(IsEnum(j, "detail4::h"));
        REQUIRE(IsEnum(j, "detail4::j"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsEnum;

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("a")));
        REQUIRE_THAT(src, IsClass(_A("b")));
        REQUIRE_THAT(src, IsClass(_A("detail::c")));
        REQUIRE_THAT(src, IsClass(_A("detail::d")));
        REQUIRE_THAT(src, IsClass(_A("detail::e")));
        REQUIRE_THAT(src, IsClass(_A("f")));
        REQUIRE_THAT(src, IsClass(_A("g")));

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("detail2::C")));
        REQUIRE_THAT(src, IsClass(_A("detail2::detail3::D")));
        REQUIRE_THAT(src, IsClass(_A("detail2::detail3::E")));
        REQUIRE_THAT(src, IsClass(_A("detail2::F")));
        REQUIRE_THAT(src, IsClass(_A("G")));

        REQUIRE_THAT(src, IsEnum(_A("detail4::i")));
        REQUIRE_THAT(src, IsEnum(_A("detail4::h")));
        REQUIRE_THAT(src, IsEnum(_A("detail4::j")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}