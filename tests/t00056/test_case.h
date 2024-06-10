/**
 * tests/t00056/test_case.h
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

TEST_CASE("t00056")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00056", "t00056_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsConcept(src, "greater_than_simple<T,L>"));
        REQUIRE(IsConcept(src, "greater_than_with_requires<T,P>"));
        REQUIRE(IsConcept(src, "max_four_bytes<T>"));
        REQUIRE(IsConcept(src, "iterable<T>"));
        REQUIRE(IsConcept(src, "has_value_type<T>"));
        REQUIRE(IsConcept(src, "convertible_to_string<T>"));
        REQUIRE(IsConcept(src, "iterable_with_value_type<T>"));
        REQUIRE(IsConcept(src, "iterable_or_small_value_type<T>"));
        REQUIRE(IsConceptParameterList(
            src, "greater_than_with_requires<T,P>", "(T l,P r)"));
        REQUIRE(IsConceptRequirement(
            src, "greater_than_with_requires<T,P>", "sizeof (l) > sizeof (r)"));

        REQUIRE(IsConceptRequirement(src, "iterable<T>", "container.begin()"));
        REQUIRE(IsConceptRequirement(src, "iterable<T>", "container.end()"));

#if (LLVM_VERSION_MAJOR == 13) || (LLVM_VERSION_MAJOR == 14)
        REQUIRE(IsConceptRequirement(
            src, "convertible_to_string<T>", "std::string({s})"));
#else
        REQUIRE(
            IsConceptRequirement(
                src,"convertible_to_string<T>", "std::string{s}"));
#endif
        REQUIRE(IsConceptRequirement(
            src, "convertible_to_string<T>", "{std::to_string(s)} noexcept"));
        REQUIRE(IsConceptRequirement(src, "convertible_to_string<T>",
            "{std::to_string(s)} -> std::same_as<std::string>"));

        REQUIRE(IsClassTemplate(src, "A<max_four_bytes T>"));

        REQUIRE(IsClassTemplate(src, "B<T>"));
        REQUIRE(IsClassTemplate(src, "C<convertible_to_string T>"));
        REQUIRE(IsClassTemplate(src, "D<iterable T1,T2,iterable T3,T4,T5>"));
        REQUIRE(IsClassTemplate(src, "E<T1,T2,T3>"));
        REQUIRE(IsClassTemplate(src, "F<T1,T2,T3>"));

        REQUIRE(
            IsConstraint(src, "A<max_four_bytes T>", "max_four_bytes<T>", "T"));

        REQUIRE(IsConstraint(src, "D<iterable T1,T2,iterable T3,T4,T5>",
            "max_four_bytes<T>", "T2"));
        REQUIRE(IsConstraint(src, "D<iterable T1,T2,iterable T3,T4,T5>",
            "max_four_bytes<T>", "T5"));
        REQUIRE(IsConstraint(
            src, "D<iterable T1,T2,iterable T3,T4,T5>", "iterable<T>", "T1"));
        REQUIRE(IsConstraint(
            src, "D<iterable T1,T2,iterable T3,T4,T5>", "iterable<T>", "T3"));

        REQUIRE(IsConstraint(
            src, "iterable_with_value_type<T>", "has_value_type<T>", "T"));

        REQUIRE(IsConstraint(
            src, "iterable_or_small_value_type<T>", "max_four_bytes<T>", "T"));
        REQUIRE(IsConstraint(src, "iterable_or_small_value_type<T>",
            "iterable_with_value_type<T>", "T"));

        REQUIRE(IsConstraint(
            src, "E<T1,T2,T3>", "greater_than_with_requires<T,P>", "T1,T3"));

        REQUIRE(IsConstraint(
            src, "F<T1,T2,T3>", "greater_than_simple<T,L>", "T1,T3"));
    });
}