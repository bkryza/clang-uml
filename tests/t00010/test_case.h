/**
 * tests/t00010/test_case.cc
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

TEST_CASE("t00010", "[test-case][class]")
{
    auto [config, db] = load_config("t00010");

    auto diagram = config.diagrams["t00010_class"];

    REQUIRE(diagram->name == "t00010_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00010_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));
        REQUIRE_THAT(src, IsClassTemplate("A", "T,P"));
        REQUIRE_THAT(src, IsClassTemplate("B", "T"));

        REQUIRE_THAT(src, (IsField<Public>("astring", "A<T,std::string>")));
        REQUIRE_THAT(src, (IsField<Public>("aintstring", "B<int>")));

        REQUIRE_THAT(
            src, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
        REQUIRE_THAT(src, IsInstantiation(_A("B<T>"), _A("B<int>")));

        REQUIRE_THAT(
            src, IsAggregation(_A("B<T>"), _A("A<T,std::string>"), "+astring"));
        REQUIRE_THAT(src, IsAggregation(_A("C"), _A("B<int>"), "+aintstring"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClassTemplate(j, "A<T,P>"));
        REQUIRE(IsClassTemplate(j, "B<T>"));
        REQUIRE(IsClass(j, "B<int>"));
        REQUIRE(IsClass(j, "A<T,std::string>"));
        REQUIRE(IsClass(j, "B<int>"));

        REQUIRE(IsField(j, "C", "aintstring", "B<int>"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsField;

        REQUIRE_THAT(src, IsClass(_A("A<T,P>")));
        REQUIRE_THAT(src, IsClass(_A("B<T>")));

        REQUIRE_THAT(src, (IsField<Public>("astring", "A<T,std::string>")));
        REQUIRE_THAT(src, (IsField<Public>("aintstring", "B<int>")));

        REQUIRE_THAT(
            src, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
        REQUIRE_THAT(src, IsInstantiation(_A("B<T>"), _A("B<int>")));

        REQUIRE_THAT(
            src, IsAggregation(_A("B<T>"), _A("A<T,std::string>"), "+astring"));
        REQUIRE_THAT(src, IsAggregation(_A("C"), _A("B<int>"), "+aintstring"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}