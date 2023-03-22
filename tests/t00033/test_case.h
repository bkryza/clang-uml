/**
 * tests/t00033/test_case.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t00033", "[test-case][class]")
{
    auto [config, db] = load_config("t00033");

    auto diagram = config.diagrams["t00033_class"];

    REQUIRE(diagram->name == "t00033_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00033_class");
    REQUIRE(model->should_include("clanguml::t00033::A"));

    {
        auto puml = generate_class_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        REQUIRE_THAT(puml, IsClassTemplate("A", "T"));
        REQUIRE_THAT(puml, IsClassTemplate("B", "T"));
        REQUIRE_THAT(puml, IsClassTemplate("C", "T"));
        REQUIRE_THAT(puml, IsClass(_A("D")));
        REQUIRE_THAT(puml, IsClass(_A("R")));

        REQUIRE_THAT(puml,
            IsDependency(_A("A<B<std::unique_ptr<C<D>>>>"),
                _A("B<std::unique_ptr<C<D>>>")));
        REQUIRE_THAT(
            puml, IsDependency(_A("B<std::unique_ptr<C<D>>>"), _A("C<D>")));
        REQUIRE_THAT(puml, IsDependency(_A("C<D>"), _A("D")));

        REQUIRE_THAT(puml, IsInstantiation(_A("C<T>"), _A("C<D>")));
        REQUIRE_THAT(
            puml, IsInstantiation(_A("B<T>"), _A("B<std::unique_ptr<C<D>>>")));
        REQUIRE_THAT(puml,
            IsInstantiation(_A("A<T>"), _A("A<B<std::unique_ptr<C<D>>>>")));

        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j,
            "A<clanguml::t00033::B<std::unique_ptr<clanguml::t00033::C<"
            "clanguml::t00033::D>>>>"));
        REQUIRE(IsDependency(j,
            "A<clanguml::t00033::B<std::unique_ptr<clanguml::t00033::C<"
            "clanguml::t00033::D>>>>",
            "B<std::unique_ptr<clanguml::t00033::C<clanguml::t00033::D>>>"));

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}
