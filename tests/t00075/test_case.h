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

TEST_CASE("t00075")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00075", "t00075_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, {"ns1::ns2", "A"}));
        REQUIRE(IsClass(src, {"ns1::ns2", "B"}));
        REQUIRE(IsClass(src, {"ns1::ns2", "ABE<ns1::ns2::C T>"}));
        REQUIRE(IsClass(src, {"ns1::ns2", "R"}));

        REQUIRE(IsEnum(src, {"ns1::ns2", "E"}));

        REQUIRE(IsConcept(src, {"ns1::ns2", "C<T>"}));

        REQUIRE(IsConceptRequirement(src, "C<T>", "T{}"));
        REQUIRE(IsConceptRequirement(src, "C<T>", "t.e()"));
        REQUIRE(IsConceptParameterList(src, "C<T>", "(T t)"));
        REQUIRE(!IsConceptParameterList(src, "C<T>", "(T ns1::ns2::t)"));

        REQUIRE(IsConstraint(src, {"ns1::ns2", "ABE<ns1::ns2::C T>"},
            {"ns1::ns2", "C<T>"}, "T", "up[#green,dashed,thickness=2]"));
    });
}