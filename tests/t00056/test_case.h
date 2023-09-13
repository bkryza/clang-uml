/**
 * tests/t00056/test_case.h
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

TEST_CASE("t00056", "[test-case][class]")
{
    auto [config, db] = load_config("t00056");

    auto diagram = config.diagrams["t00056_class"];

    REQUIRE(diagram->name == "t00056_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00056_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsConcept(_A("greater_than_simple<T,L>")));
        REQUIRE_THAT(src, IsConcept(_A("greater_than_with_requires<T,P>")));
        REQUIRE_THAT(src, IsConcept(_A("max_four_bytes<T>")));
        REQUIRE_THAT(src, IsConcept(_A("iterable<T>")));
        REQUIRE_THAT(src, IsConcept(_A("has_value_type<T>")));
        REQUIRE_THAT(src, IsConcept(_A("convertible_to_string<T>")));
        REQUIRE_THAT(src, IsConcept(_A("iterable_with_value_type<T>")));
        REQUIRE_THAT(src, IsConcept(_A("iterable_or_small_value_type<T>")));

        REQUIRE_THAT(src,
            IsConceptRequirement(_A("greater_than_with_requires<T,P>"),
                "sizeof (l) > sizeof (r)"));

        REQUIRE_THAT(
            src, IsConceptRequirement(_A("iterable<T>"), "container.begin()"));
        REQUIRE_THAT(
            src, IsConceptRequirement(_A("iterable<T>"), "container.end()"));

#ifdef _MSC_VER
        REQUIRE_THAT(src,
            IsConceptRequirement(
                _A("convertible_to_string<T>"), "std::string{s}"));
#else
        REQUIRE_THAT(src,
            IsConceptRequirement(
                _A("convertible_to_string<T>"), "std::string{s}"));
#endif
        REQUIRE_THAT(src,
            IsConceptRequirement(_A("convertible_to_string<T>"),
                "{std::to_string(s)} noexcept"));
        REQUIRE_THAT(src,
            IsConceptRequirement(_A("convertible_to_string<T>"),
                "{std::to_string(s)} -> std::same_as<std::string>"));

        // Check if class templates exist
        REQUIRE_THAT(src, IsClassTemplate("A", "max_four_bytes T"));
        REQUIRE_THAT(src, IsClassTemplate("B", "T"));
        REQUIRE_THAT(src, IsClassTemplate("C", "convertible_to_string T"));
        REQUIRE_THAT(
            src, IsClassTemplate("D", "iterable T1,T2,iterable T3,T4,T5"));
        REQUIRE_THAT(src, IsClassTemplate("E", "T1,T2,T3"));
        REQUIRE_THAT(src, IsClassTemplate("F", "T1,T2,T3"));

        // Check if all relationships exist
        REQUIRE_THAT(src,
            IsConstraint(
                _A("A<max_four_bytes T>"), _A("max_four_bytes<T>"), "T"));

        REQUIRE_THAT(src,
            IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
                _A("max_four_bytes<T>"), "T2"));
        REQUIRE_THAT(src,
            IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
                _A("max_four_bytes<T>"), "T5"));
        REQUIRE_THAT(src,
            IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
                _A("iterable<T>"), "T1"));
        REQUIRE_THAT(src,
            IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
                _A("iterable<T>"), "T3"));

        REQUIRE_THAT(src,
            IsConstraint(_A("iterable_with_value_type<T>"),
                _A("has_value_type<T>"), "T"));

        REQUIRE_THAT(src,
            IsConstraint(_A("iterable_or_small_value_type<T>"),
                _A("max_four_bytes<T>"), "T"));
        REQUIRE_THAT(src,
            IsConstraint(_A("iterable_or_small_value_type<T>"),
                _A("iterable_with_value_type<T>"), "T"));

        REQUIRE_THAT(src,
            IsConstraint(_A("E<T1,T2,T3>"),
                _A("greater_than_with_requires<T,P>"), "T1,T3"));

        REQUIRE_THAT(src,
            IsConstraint(
                _A("F<T1,T2,T3>"), _A("greater_than_simple<T,L>"), "T1,T3"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsConcept(j, "greater_than_simple<T,L>"));
        REQUIRE(IsConcept(j, "greater_than_with_requires<T,P>"));
        REQUIRE(IsConcept(j, "max_four_bytes<T>"));
        REQUIRE(IsConcept(j, "iterable<T>"));
        REQUIRE(IsConcept(j, "has_value_type<T>"));
        REQUIRE(IsConcept(j, "convertible_to_string<T>"));
        REQUIRE(IsConcept(j, "iterable_with_value_type<T>"));
        REQUIRE(IsConcept(j, "iterable_or_small_value_type<T>"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsConcept;
        using mermaid::IsConceptRequirement;
        using mermaid::IsConstraint;

        // Check if all classes exist
        REQUIRE_THAT(src, IsConcept(_A("greater_than_simple<T,L>")));
        REQUIRE_THAT(src, IsConcept(_A("greater_than_with_requires<T,P>")));
        REQUIRE_THAT(src, IsConcept(_A("max_four_bytes<T>")));
        REQUIRE_THAT(src, IsConcept(_A("iterable<T>")));
        REQUIRE_THAT(src, IsConcept(_A("has_value_type<T>")));
        REQUIRE_THAT(src, IsConcept(_A("convertible_to_string<T>")));
        REQUIRE_THAT(src, IsConcept(_A("iterable_with_value_type<T>")));
        REQUIRE_THAT(src, IsConcept(_A("iterable_or_small_value_type<T>")));

        REQUIRE_THAT(src,
            IsConceptRequirement(_A("greater_than_with_requires<T,P>"),
                "sizeof (l) > sizeof (r)"));

        REQUIRE_THAT(
            src, IsConceptRequirement(_A("iterable<T>"), "container.begin()"));
        REQUIRE_THAT(
            src, IsConceptRequirement(_A("iterable<T>"), "container.end()"));

#ifdef _MSC_VER
        REQUIRE_THAT(src,
            IsConceptRequirement(
                _A("convertible_to_string<T>"), "std::string{s}"));
#else
        REQUIRE_THAT(src,
            IsConceptRequirement(
                _A("convertible_to_string<T>"), "std::string{s}"));
#endif
        REQUIRE_THAT(src,
            IsConceptRequirement(_A("convertible_to_string<T>"),
                "{std::to_string(s)} noexcept"));
        REQUIRE_THAT(src,
            IsConceptRequirement(_A("convertible_to_string<T>"),
                "{std::to_string(s)} -> std::same_as<std::string>"));

        // Check if class templates exist
        REQUIRE_THAT(src, IsClass(_A("A<max_four_bytes T>")));
        REQUIRE_THAT(src, IsClass(_A("B<T>")));
        REQUIRE_THAT(src, IsClass(_A("C<convertible_to_string T>")));
        REQUIRE_THAT(src, IsClass(_A("D<iterable T1,T2,iterable T3,T4,T5>")));
        REQUIRE_THAT(src, IsClass(_A("E<T1,T2,T3>")));
        REQUIRE_THAT(src, IsClass(_A("F<T1,T2,T3>")));

        // Check if all relationships exist
        REQUIRE_THAT(src,
            IsConstraint(
                _A("A<max_four_bytes T>"), _A("max_four_bytes<T>"), "T"));

        REQUIRE_THAT(src,
            IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
                _A("max_four_bytes<T>"), "T2"));
        REQUIRE_THAT(src,
            IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
                _A("max_four_bytes<T>"), "T5"));
        REQUIRE_THAT(src,
            IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
                _A("iterable<T>"), "T1"));
        REQUIRE_THAT(src,
            IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
                _A("iterable<T>"), "T3"));

        REQUIRE_THAT(src,
            IsConstraint(_A("iterable_with_value_type<T>"),
                _A("has_value_type<T>"), "T"));

        REQUIRE_THAT(src,
            IsConstraint(_A("iterable_or_small_value_type<T>"),
                _A("max_four_bytes<T>"), "T"));
        REQUIRE_THAT(src,
            IsConstraint(_A("iterable_or_small_value_type<T>"),
                _A("iterable_with_value_type<T>"), "T"));

        REQUIRE_THAT(src,
            IsConstraint(_A("E<T1,T2,T3>"),
                _A("greater_than_with_requires<T,P>"), "T1,T3"));

        REQUIRE_THAT(src,
            IsConstraint(
                _A("F<T1,T2,T3>"), _A("greater_than_simple<T,L>"), "T1,T3"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}