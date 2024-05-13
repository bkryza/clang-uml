/**
 * tests/t00058/test_case.h
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

TEST_CASE("t00058")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00058");

    auto diagram = config.diagrams["t00058_class"];

    REQUIRE(diagram->name == "t00058_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00058_class");

    CHECK_CLASS_DIAGRAM(
        config, diagram, *model,
        [](const auto &src) {
            REQUIRE(IsClassTemplate(src, "A<int,int,double,std::string>"));
            REQUIRE(IsClassTemplate(
                src, "B<int,std::string,int,double,A<int,int>>"));

            REQUIRE(IsConcept(src, "same_as_first_type<T,Args...>"));

            REQUIRE(IsConstraint(src, "A<T,Args...>",
                "same_as_first_type<T,Args...>", "T,Args..."));

            REQUIRE(IsConstraint(src, "B<T,P,Args...>",
                "same_as_first_type<T,Args...>", "T,Args..."));

            REQUIRE(IsAggregation<Public>(
                src, "R", "A<int,int,double,std::string>", "aa"));
            REQUIRE(IsAggregation<Public>(
                src, "R", "B<int,std::string,int,double,A<int,int>>", "bb"));

            REQUIRE(IsInstantiation(
                src, "A<T,Args...>", "A<int,int,double,std::string>"));
            REQUIRE(IsInstantiation(src, "B<T,P,Args...>",
                "B<int,std::string,int,double,A<int,int>>"));
        },
        [](const plantuml_t &src) {
            // TODO: This dependency has to be added manually in config file
            //       so it doesn't work in JSON
            REQUIRE(IsDependency(
                src, "same_as_first_type<T,Args...>", "first_type<T,Args...>"));
        },
        [](const mermaid_t &src) {
            // TODO: This dependency has to be added manually in config file
            //       so it doesn't work in JSON
            REQUIRE(IsDependency(
                src, "same_as_first_type<T,Args...>", "first_type<T,Args...>"));
        });

    /*
        {
            auto src = generate_class_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            REQUIRE_THAT(src, IsClassTemplate("A",
       "int,int,double,std::string")); REQUIRE_THAT( src, IsClassTemplate("B",
       "int,std::string,int,double,A<int,int>"));

            REQUIRE_THAT(src, IsConcept(_A("same_as_first_type<T,Args...>")));

            REQUIRE_THAT(src,
                IsConstraint(_A("A<T,Args...>"),
                    _A("same_as_first_type<T,Args...>"), "T,Args..."));

            REQUIRE_THAT(src,
                IsConstraint(_A("B<T,P,Args...>"),
                    _A("same_as_first_type<T,Args...>"), "T,Args..."));

            REQUIRE_THAT(src,
                IsAggregation(_A("R"), _A("A<int,int,double,std::string>"),
       "+aa")); REQUIRE_THAT(src, IsAggregation(_A("R"),
                    _A("B<int,std::string,int,double,A<int,int>>"), "+bb"));

            REQUIRE_THAT(src,
                IsInstantiation(
                    _A("A<T,Args...>"), _A("A<int,int,double,std::string>")));
            REQUIRE_THAT(src,
                IsInstantiation(_A("B<T,P,Args...>"),
                    _A("B<int,std::string,int,double,A<int,int>>")));

            REQUIRE_THAT(src,
                IsDependency(_A("same_as_first_type<T,Args...>"),
                    _A("first_type<T,Args...>")));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_class_json(diagram, *model);

            using namespace json;

            REQUIRE(IsClass(j, "A<int,int,double,std::string>"));
            REQUIRE(IsClass(j, "B<int,std::string,int,double,A<int,int>>"));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }
        {
            auto src = generate_class_mermaid(diagram, *model);

            mermaid::AliasMatcher _A(src);
            using mermaid::IsConcept;
            using mermaid::IsConstraint;

            REQUIRE_THAT(src, IsClass(_A("A<int,int,double,std::string>")));
            REQUIRE_THAT(
                src, IsClass(_A("B<int,std::string,int,double,A<int,int>>")));

            REQUIRE_THAT(src, IsConcept(_A("same_as_first_type<T,Args...>")));

            REQUIRE_THAT(src,
                IsConstraint(_A("A<T,Args...>"),
                    _A("same_as_first_type<T,Args...>"), "T,Args..."));

            REQUIRE_THAT(src,
                IsConstraint(_A("B<T,P,Args...>"),
                    _A("same_as_first_type<T,Args...>"), "T,Args..."));

            REQUIRE_THAT(src,
                IsAggregation(_A("R"), _A("A<int,int,double,std::string>"),
       "+aa")); REQUIRE_THAT(src, IsAggregation(_A("R"),
                    _A("B<int,std::string,int,double,A<int,int>>"), "+bb"));

            REQUIRE_THAT(src,
                IsInstantiation(
                    _A("A<T,Args...>"), _A("A<int,int,double,std::string>")));
            REQUIRE_THAT(src,
                IsInstantiation(_A("B<T,P,Args...>"),
                    _A("B<int,std::string,int,double,A<int,int>>")));

            // TODO
            //        REQUIRE_THAT(src,
            //            IsDependency(_A("same_as_first_type<T,Args...>"),
            //                _A("first_type<T,Args...>")));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }*/
}