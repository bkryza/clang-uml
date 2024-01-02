/**
 * tests/t00019/test_case.cc
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

TEST_CASE("t00019", "[test-case][class]")
{
    auto [config, db] = load_config("t00019");

    auto diagram = config.diagrams["t00019_class"];

    REQUIRE(diagram->name == "t00019_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00019_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));
        REQUIRE_THAT(src, IsClass(_A("Base")));
        REQUIRE_THAT(src, IsClassTemplate("Layer1", "LowerLayer"));
        REQUIRE_THAT(src, IsClassTemplate("Layer2", "LowerLayer"));
        REQUIRE_THAT(src, IsClassTemplate("Layer3", "LowerLayer"));

        REQUIRE_THAT(src, IsBaseClass(_A("Base"), _A("Layer3<Base>")));
        REQUIRE_THAT(src, !IsDependency(_A("Base"), _A("Layer3<Base>")));

        REQUIRE_THAT(
            src, IsBaseClass(_A("Layer3<Base>"), _A("Layer2<Layer3<Base>>")));
        REQUIRE_THAT(
            src, !IsDependency(_A("Layer3<Base>"), _A("Layer2<Layer3<Base>>")));

        REQUIRE_THAT(src,
            IsBaseClass(_A("Layer2<Layer3<Base>>"),
                _A("Layer1<Layer2<Layer3<Base>>>")));

        REQUIRE_THAT(src,
            !IsDependency(_A("Layer2<Layer3<Base>>"),
                _A("Layer1<Layer2<Layer3<Base>>>")));

        REQUIRE_THAT(src,
            IsAggregation(
                _A("A"), _A("Layer1<Layer2<Layer3<Base>>>"), "+layers"));
        REQUIRE_THAT(
            src, !IsDependency(_A("A"), _A("Layer1<Layer2<Layer3<Base>>>")));

        REQUIRE_THAT(src,
            !IsAggregation(_A("A"), _A("Layer2<Layer3<Base>>"), "+layers"));

        REQUIRE_THAT(
            src, !IsAggregation(_A("A"), _A("Layer3<Base>"), "+layers"));

        REQUIRE_THAT(src, !IsAggregation(_A("A"), _A("Base"), "+layers"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "Base"));
        REQUIRE(IsClassTemplate(j, "Layer1<LowerLayer>"));
        REQUIRE(IsClassTemplate(j, "Layer2<LowerLayer>"));
        REQUIRE(IsClassTemplate(j, "Layer3<LowerLayer>"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);

        REQUIRE_THAT(src, IsClass(_A("Base")));
        REQUIRE_THAT(src, IsClass(_A("Layer1<LowerLayer>")));
        REQUIRE_THAT(src, IsClass(_A("Layer2<LowerLayer>")));
        REQUIRE_THAT(src, IsClass(_A("Layer3<LowerLayer>")));

        REQUIRE_THAT(src, IsBaseClass(_A("Base"), _A("Layer3<Base>")));
        REQUIRE_THAT(src, !IsDependency(_A("Base"), _A("Layer3<Base>")));

        REQUIRE_THAT(
            src, IsBaseClass(_A("Layer3<Base>"), _A("Layer2<Layer3<Base>>")));
        REQUIRE_THAT(
            src, !IsDependency(_A("Layer3<Base>"), _A("Layer2<Layer3<Base>>")));

        REQUIRE_THAT(src,
            IsBaseClass(_A("Layer2<Layer3<Base>>"),
                _A("Layer1<Layer2<Layer3<Base>>>")));

        REQUIRE_THAT(src,
            !IsDependency(_A("Layer2<Layer3<Base>>"),
                _A("Layer1<Layer2<Layer3<Base>>>")));

        REQUIRE_THAT(src,
            IsAggregation(
                _A("A"), _A("Layer1<Layer2<Layer3<Base>>>"), "+layers"));
        REQUIRE_THAT(
            src, !IsDependency(_A("A"), _A("Layer1<Layer2<Layer3<Base>>>")));

        REQUIRE_THAT(src,
            !IsAggregation(_A("A"), _A("Layer2<Layer3<Base>>"), "+layers"));

        REQUIRE_THAT(
            src, !IsAggregation(_A("A"), _A("Layer3<Base>"), "+layers"));

        REQUIRE_THAT(src, !IsAggregation(_A("A"), _A("Base"), "+layers"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}