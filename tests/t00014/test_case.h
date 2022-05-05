/**
 * tests/t00014/test_case.h
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t00014", "[test-case][class]")
{
    auto [config, db] = load_config("t00014");

    auto diagram = config.diagrams["t00014_class"];

    REQUIRE(diagram->name == "t00014_class");

    auto model = generate_class_diagram(db, diagram);

    REQUIRE(model->name() == "t00014_class");
    REQUIRE(model->should_include("clanguml::t00014::B"));

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "T,P"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "T,std::string"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "T,std::unique_ptr<std::string>"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "double,T"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "long,U"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "long,T"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "long,bool"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "double,bool"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "long,float"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "double,float"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "bool,std::string"));
    REQUIRE_THAT(puml, IsClass(_A("B")));

    REQUIRE_THAT(
        puml, !IsClassTemplate("std::std::function", "void(T...,int),int)"));

    REQUIRE_THAT(puml, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<long,T>"), _A("A<long,float>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<long,T>"), _A("A<long,bool>")));
    // TODO: Fix matching partial template specializations with differently
    //       named template paremeters
    // REQUIRE_THAT(
    //        puml, IsInstantiation(_A("A<T,P>"), _A("A<long,T>")));
    REQUIRE_THAT(
        puml, IsInstantiation(_A("A<double,T>"), _A("A<double,float>")));
    REQUIRE_THAT(
        puml, IsInstantiation(_A("A<double,T>"), _A("A<double,bool>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<T,P>"), _A("A<double,T>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
    REQUIRE_THAT(puml,
        IsInstantiation(_A("A<T,std::string>"), _A("A<bool,std::string>")));

    REQUIRE_THAT(puml,
        IsInstantiation(_A("A<T,std::unique_ptr<std::string>>"),
            _A("A<float,std::unique_ptr<std::string>>")));
    REQUIRE_THAT(puml,
        IsInstantiation(_A("A<T,P>"), _A("A<T,std::unique_ptr<std::string>>")));

    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("B"), "+vps"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("B"), "-bapair"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("R"), _A("A<long,float>"), "-aboolfloat"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("A<long,bool>"), "-bapair"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("R"), _A("A<double,bool>"), "-aboolfloat"));
    REQUIRE_THAT(puml, IsAssociation(_A("R"), _A("A<double,float>"), "-afloat"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("R"), _A("A<bool,std::string>"), "-boolstring"));
    REQUIRE_THAT(puml,
        IsAggregation(_A("R"), _A("A<float,std::unique_ptr<std::string>>"),
            "-floatstring"));

    save_puml(
        "./" + config.output_directory() + "/" + diagram->name + ".puml", puml);
}
