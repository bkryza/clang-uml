/**
 * tests/t00031/test_case.cc
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

TEST_CASE("t00031", "[test-case][class]")
{
    auto [config, db] = load_config("t00031");

    auto diagram = config.diagrams["t00031_class"];

    REQUIRE(diagram->name == "t00031_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00031_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsEnum(_A("B")));
        REQUIRE_THAT(src, IsClassTemplate("C", "T"));
        REQUIRE_THAT(src, IsClass(_A("D")));

        REQUIRE_THAT(src,
            IsAssociation(
                _A("R"), _A("A"), "+aaa", "", "", "[#red,dashed,thickness=2]"));
        REQUIRE_THAT(src,
            IsComposition(_A("R"), _A("B"), "+bbb", "", "",
                "[#green,dashed,thickness=4]"));
        REQUIRE_THAT(src, IsDependency(_A("R"), _A("B")));
        REQUIRE_THAT(src,
            IsAggregation(_A("R"), _A("C<int>"), "+ccc", "", "",
                "[#blue,dotted,thickness=8]"));
        REQUIRE_THAT(src,
            IsAssociation(_A("R"), _A("D"), "+ddd", "", "",
                "[#blue,plain,thickness=16]"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClassTemplate(j, "C<T>"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsEnum;

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsEnum(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C<T>")));
        REQUIRE_THAT(src, IsClass(_A("D")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}