/**
 * tests/t00062/test_case.h
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

TEST_CASE("t00062", "[test-case][class]")
{
    auto [config, db] = load_config("t00062");

    auto diagram = config.diagrams["t00062_class"];

    REQUIRE(diagram->name == "t00062_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00062_class");

    {
        auto puml = generate_class_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        
    // Check if all classes exist
    REQUIRE_THAT(puml, IsClass(_A("AAA")));
    
    // Check if class templates exist
    //REQUIRE_THAT(puml, IsClassTemplate("A", "T,P,CMP,int N"));
    
    // Check concepts
    //REQUIRE_THAT(puml, IsConcept(_A("AConcept<T>")));
    //REQUIRE_THAT(puml,
    //    IsConceptRequirement(
    //        _A("AConcept<T,P>"), "sizeof (T) > sizeof (P)"));

    // Check if all enums exist
    //REQUIRE_THAT(puml, IsEnum(_A("Lights")));
    
    // Check if all inner classes exist
    //REQUIRE_THAT(puml, IsInnerClass(_A("A"), _A("AA")));

    // Check if all inheritance relationships exist
    //REQUIRE_THAT(puml, IsBaseClass(_A("Base"), _A("Child")));
    
    // Check if all methods exist
    //REQUIRE_THAT(puml, (IsMethod<Public, Const>("foo")));
    
    // Check if all fields exist
    //REQUIRE_THAT(puml, (IsField<Private>("private_member", "int")));
    
    // Check if all relationships exist
    //REQUIRE_THAT(puml, IsAssociation(_A("D"), _A("A"), "-as"));
    //REQUIRE_THAT(puml, IsDependency(_A("R"), _A("B")));
    //REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("D"), "-ag"));
    //REQUIRE_THAT(puml, IsComposition(_A("R"), _A("D"), "-ac"));
    //REQUIRE_THAT(puml, IsInstantiation(_A("ABCD::F<T>"), _A("F<int>")));


        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }

}