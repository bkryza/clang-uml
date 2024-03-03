/**
 * tests/t00075/test_case.h
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

TEST_CASE("t00075", "[test-case][class]")
{
    auto [config, db] = load_config("t00075");

    auto diagram = config.diagrams["t00075_class"];

    REQUIRE(diagram->name == "t00075_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00075_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("ABE<ns1::ns2::C T>")));
        REQUIRE_THAT(src, IsClass(_A("R")));

        REQUIRE_THAT(src, IsEnum(_A("E")));

        REQUIRE_THAT(src, IsConcept(_A("C<T>")));

        REQUIRE_THAT(src, IsConceptRequirement(_A("C<T>"), "T{}"));
        REQUIRE_THAT(src, IsConceptRequirement(_A("C<T>"), "t.e()"));
        REQUIRE_THAT(src, IsConceptRequirement(_A("C<T>"), "(T t)"));
        REQUIRE_THAT(src, !IsConceptRequirement(_A("C<T>"), "(T ns1::ns2::t)"));

        REQUIRE_THAT(src,
            IsConstraint(_A("ABE<ns1::ns2::C T>"), _A("C<T>"), "T",
                "up[#green,dashed,thickness=2]"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsClass;
        using mermaid::IsConcept;
        using mermaid::IsConceptRequirement;
        using mermaid::IsEnum;

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("ns1::ns2::A")));
        REQUIRE_THAT(src, IsClass(_A("ns1::ns2::B")));
        REQUIRE_THAT(src, IsClass(_A("ns1::ns2::ABE<ns1::ns2::C T>")));
        REQUIRE_THAT(src, IsClass(_A("ns1::ns2::R")));

        REQUIRE_THAT(src, IsEnum(_A("ns1::ns2::E")));

        REQUIRE_THAT(src, IsConcept(_A("ns1::ns2::C<T>")));

        REQUIRE_THAT(src, IsConceptRequirement(_A("ns1::ns2::C<T>"), "T{}"));
        REQUIRE_THAT(src, IsConceptRequirement(_A("ns1::ns2::C<T>"), "t.e()"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}