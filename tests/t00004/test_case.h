/**
 * tests/t00004/test_case.cc
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

TEST_CASE("t00004", "[test-case][class]")
{
    auto [config, db] = load_config("t00004");

    auto diagram = config.diagrams["t00004_class"];

    REQUIRE(diagram->name == "t00004_class");

    REQUIRE(diagram->include().namespaces.size() == 1);
    REQUIRE(diagram->exclude().namespaces.size() == 0);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00004_class");
    REQUIRE(!model->should_include("std::vector"));
    REQUIRE(model->should_include("clanguml::t00004::A"));
    REQUIRE(model->should_include("clanguml::t00004::A::AA"));
    REQUIRE(model->should_include("clanguml::t00004::A:::AAA"));

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClass(_A("A")));
    REQUIRE_THAT(puml, IsClass(_A("A::AA")));
    REQUIRE_THAT(puml, IsClass(_A("A::AA::AAA")));
    REQUIRE_THAT(puml, IsEnum(_A("B::AA")));
    REQUIRE_THAT(puml, IsEnum(_A("A::AA::Lights")));
    REQUIRE_THAT(puml, IsInnerClass(_A("A"), _A("A::AA")));
    REQUIRE_THAT(puml, IsInnerClass(_A("A::AA"), _A("A::AA::AAA")));
    REQUIRE_THAT(puml, IsInnerClass(_A("A::AA"), _A("A::AA::Lights")));

    REQUIRE_THAT(puml, (IsMethod<Public, Const>("foo")));
    REQUIRE_THAT(puml, (IsMethod<Public, Const>("foo2")));

    REQUIRE_THAT(puml, IsClassTemplate("C", "T"));
    REQUIRE_THAT(puml, IsInnerClass(_A("C<T>"), _A("C::AA")));
    REQUIRE_THAT(puml, IsInnerClass(_A("C::AA"), _A("C::AA::AAA")));
    REQUIRE_THAT(puml, IsInnerClass(_A("C<T>"), _A("C::CC")));
    REQUIRE_THAT(puml, IsInnerClass(_A("C::AA"), _A("C::AA::CCC")));

    REQUIRE_THAT(puml, IsInnerClass(_A("C<T>"), _A("C::B<V>")));
    REQUIRE_THAT(puml, IsAggregation(_A("C<T>"), _A("C::B<int>"), "+b_int"));
    REQUIRE_THAT(puml, !IsInnerClass(_A("C<T>"), _A("C::B")));
    REQUIRE_THAT(puml, IsInstantiation(_A("C::B<V>"), _A("C::B<int>")));

    REQUIRE_THAT(puml, IsClass(_A("detail::D")));
    REQUIRE_THAT(puml, IsClass(_A("detail::D::DD")));
    REQUIRE_THAT(puml, IsEnum(_A("detail::D::AA")));

    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);
}
