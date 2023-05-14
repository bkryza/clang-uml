/**
 * tests/t00064/test_case.h
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

TEST_CASE("t00064", "[test-case][class]")
{
    auto [config, db] = load_config("t00064");

    auto diagram = config.diagrams["t00064_class"];

    REQUIRE(diagram->name == "t00064_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00064_class");

    {
        auto puml = generate_class_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));

        REQUIRE_THAT(puml, !Contains("type-parameter-"));

        REQUIRE_THAT(puml, IsClass(_A("A")));
        REQUIRE_THAT(puml, IsClass(_A("B")));
        REQUIRE_THAT(puml, IsClass(_A("C")));
        REQUIRE_THAT(puml, IsClass(_A("R")));

        REQUIRE_THAT(puml, IsClassTemplate("type_list", "Ts..."));
        REQUIRE_THAT(puml, IsClassTemplate("type_list", "Ret(Arg &&),Ts..."));
        REQUIRE_THAT(puml, IsClassTemplate("type_list", "T const,Ts..."));

        REQUIRE_THAT(puml, IsClassTemplate("head", "typename"));
        REQUIRE_THAT(puml, IsClassTemplate("head", "type_list<Head,Tail...>"));
        REQUIRE_THAT(
            puml, IsClassTemplate("type_group_pair", "typename,typename"));
        REQUIRE_THAT(puml,
            IsClassTemplate(
                "type_group_pair", "type_list<First...>,type_list<Second...>"));
        REQUIRE_THAT(puml,
            IsClassTemplate(
                "type_group_pair", "type_list<float,double>,type_list<A,B,C>"));

        REQUIRE_THAT(puml, IsClassTemplate("optional_ref", "T"));

        REQUIRE_THAT(puml,
            IsClassTemplate("type_group_pair_it",
                "It,type_list<First...>,type_list<Second...>"));
        REQUIRE_THAT(
            puml, (IsMethod<Public>("get", "ref_t", "unsigned int i")));
#if LLVM_VERSION_MAJOR < 16
        REQUIRE_THAT(puml,
            (IsMethod<Public>("getp", "value_type const*", "unsigned int i")));
        REQUIRE_THAT(puml,
            (IsMethod<Public>("find", "unsigned int", "value_type const& v")));
#else
        REQUIRE_THAT(puml,
            (IsMethod<Public>("getp", "const value_type *", "unsigned int i")));
        REQUIRE_THAT(puml,
            (IsMethod<Public>("find", "unsigned int", "const value_type & v")));

#endif
        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}