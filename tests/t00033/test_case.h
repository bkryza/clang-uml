/**
 * tests/t00033/test_case.cc
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

TEST_CASE("t00033", "[test-case][class]")
{
    auto [config, db] = load_config("t00033");

    auto diagram = config.diagrams["t00033_class"];

    REQUIRE(diagram->name == "t00033_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00033_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsClassTemplate("A", "T"));
        REQUIRE_THAT(src, IsClassTemplate("B", "T"));
        REQUIRE_THAT(src, IsClassTemplate("C", "T"));
        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, IsClass(_A("R")));

        REQUIRE_THAT(src,
            IsDependency(_A("A<B<std::unique_ptr<C<D>>>>"),
                _A("B<std::unique_ptr<C<D>>>")));
        REQUIRE_THAT(
            src, IsDependency(_A("B<std::unique_ptr<C<D>>>"), _A("C<D>")));
        REQUIRE_THAT(src, IsDependency(_A("C<D>"), _A("D")));

        REQUIRE_THAT(src, IsInstantiation(_A("C<T>"), _A("C<D>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("B<T>"), _A("B<std::unique_ptr<C<D>>>")));
        REQUIRE_THAT(src,
            IsInstantiation(_A("A<T>"), _A("A<B<std::unique_ptr<C<D>>>>")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A<B<std::unique_ptr<C<D>>>>"));
        REQUIRE(IsDependency(
            j, "A<B<std::unique_ptr<C<D>>>>", "B<std::unique_ptr<C<D>>>"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);

        REQUIRE_THAT(src, IsClass(_A("A<T>")));
        REQUIRE_THAT(src, IsClass(_A("B<T>")));
        REQUIRE_THAT(src, IsClass(_A("C<T>")));
        REQUIRE_THAT(src, IsClass(_A("D")));
        REQUIRE_THAT(src, IsClass(_A("R")));

        REQUIRE_THAT(src,
            IsDependency(_A("A<B<std::unique_ptr<C<D>>>>"),
                _A("B<std::unique_ptr<C<D>>>")));
        REQUIRE_THAT(
            src, IsDependency(_A("B<std::unique_ptr<C<D>>>"), _A("C<D>")));
        REQUIRE_THAT(src, IsDependency(_A("C<D>"), _A("D")));

        REQUIRE_THAT(src, IsInstantiation(_A("C<T>"), _A("C<D>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("B<T>"), _A("B<std::unique_ptr<C<D>>>")));
        REQUIRE_THAT(src,
            IsInstantiation(_A("A<T>"), _A("A<B<std::unique_ptr<C<D>>>>")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}