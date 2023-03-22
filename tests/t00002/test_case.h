/**
 * tests/t00002/test_case.cc
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

TEST_CASE("t00002", "[test-case][class]")
{
    auto [config, db] = load_config("t00002");

    auto diagram = config.diagrams["t00002_class"];

    REQUIRE(diagram->name == "t00002_class");

    REQUIRE(diagram->include().namespaces.size() == 1);
    REQUIRE_THAT(diagram->include().namespaces,
        VectorContains(
            clanguml::common::model::namespace_{"clanguml::t00002"}));

    REQUIRE(diagram->exclude().namespaces.size() == 0);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00002_class");

    REQUIRE(model->should_include({"clanguml", "t00002"}, "A"));
    REQUIRE(!model->should_include({"std"}, "vector"));

    {
        auto puml = generate_class_puml(diagram, *model);
        AliasMatcher _A(puml);

        REQUIRE_THAT(puml, StartsWith("@startuml"));
        REQUIRE_THAT(puml, EndsWith("@enduml\n"));
        REQUIRE_THAT(puml, IsAbstractClass(_A("A")));
        REQUIRE_THAT(puml, IsClass(_A("B")));
        REQUIRE_THAT(puml, IsClass(_A("C")));
        REQUIRE_THAT(puml, IsClass(_A("D")));
        REQUIRE_THAT(puml, IsBaseClass(_A("A"), _A("B")));
        REQUIRE_THAT(puml, IsBaseClass(_A("A"), _A("C")));
        REQUIRE_THAT(puml, IsBaseClass(_A("B"), _A("D")));
        REQUIRE_THAT(puml, IsBaseClass(_A("C"), _A("D")));
        REQUIRE_THAT(puml, (IsMethod<Public, Abstract>("foo_a")));
        REQUIRE_THAT(puml, (IsMethod<Public, Abstract>("foo_c")));

        REQUIRE_THAT(puml, IsAssociation(_A("D"), _A("A"), "-as"));

        REQUIRE_THAT(puml, HasNote(_A("A"), "left", "This is class A"));
        REQUIRE_THAT(puml, HasNote(_A("B"), "top", "This is class B"));

        REQUIRE_THAT(puml,
            HasLink(_A("A"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t00002/t00002.cc#L7",
                    clanguml::util::get_git_commit()),
                "This is class A"));

        REQUIRE_THAT(puml,
            HasLink(_A("B"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t00002/t00002.cc#L16",
                    clanguml::util::get_git_commit()),
                "This is class B"));

        save_puml(
            config.output_directory() + "/" + diagram->name + ".puml", puml);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClass(j, "B"));
        REQUIRE(IsClass(j, "C"));
        REQUIRE(IsBaseClass(j, "A", "B"));
        REQUIRE(IsBaseClass(j, "A", "C"));
        REQUIRE(IsBaseClass(j, "B", "D"));
        REQUIRE(IsBaseClass(j, "C", "D"));
        REQUIRE(IsMethod(j, "A", "foo_a"));
        REQUIRE(IsMethod(j, "C", "foo_c"));
        REQUIRE(IsField(j, "E", "as", "std::vector<A *>"));
        REQUIRE(IsAssociation(j, "D", "A", "as"));

        save_json(config.output_directory() + "/" + diagram->name + ".json", j);
    }
}