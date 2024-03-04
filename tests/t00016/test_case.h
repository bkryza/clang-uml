/**
 * tests/t00016/test_case.cc
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

TEST_CASE("t00016", "[test-case][class]")
{
    auto [config, db] = load_config("t00016");

    auto diagram = config.diagrams["t00016_class"];

    REQUIRE(diagram->name == "t00016_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00016_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));
        REQUIRE_THAT(src, IsClassTemplate("is_numeric", "typename"));
        REQUIRE_THAT(src, IsClassTemplate("is_numeric", "int"));
        REQUIRE_THAT(src, IsClassTemplate("is_numeric", "bool"));
        REQUIRE_THAT(src, IsClassTemplate("is_numeric", "char"));
        REQUIRE_THAT(src, IsClassTemplate("is_numeric", "float"));

        REQUIRE_THAT(src,
            IsInstantiation(
                _A("is_numeric<typename>"), _A("is_numeric<int>"), "up"));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("is_numeric<typename>"), _A("is_numeric<bool>"), "up"));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("is_numeric<typename>"), _A("is_numeric<char>"), "up"));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("is_numeric<typename>"), _A("is_numeric<float>"), "up"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClassTemplate(j, "is_numeric<typename>"));
        REQUIRE(IsClass(j, "is_numeric<int>"));
        REQUIRE(IsClass(j, "is_numeric<bool>"));
        REQUIRE(IsClass(j, "is_numeric<char>"));
        REQUIRE(IsClass(j, "is_numeric<float>"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);

        REQUIRE_THAT(src, IsClass(_A("is_numeric<typename>")));
        REQUIRE_THAT(src, IsClass(_A("is_numeric<int>")));
        REQUIRE_THAT(src, IsClass(_A("is_numeric<bool>")));
        REQUIRE_THAT(src, IsClass(_A("is_numeric<char>")));
        REQUIRE_THAT(src, IsClass(_A("is_numeric<float>")));

        REQUIRE_THAT(src,
            IsInstantiation(_A("is_numeric<typename>"), _A("is_numeric<int>")));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("is_numeric<typename>"), _A("is_numeric<bool>")));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("is_numeric<typename>"), _A("is_numeric<char>")));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("is_numeric<typename>"), _A("is_numeric<float>")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}