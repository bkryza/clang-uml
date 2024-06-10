/**
 * tests/t00008/test_case.h
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

TEST_CASE("t00008")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00008", "t00008_class");

    CHECK_CLASS_DIAGRAM(
        *config, diagram, *model,
        [](const auto &src) {
            // TODO: add option to resolve using declared types
            // REQUIRE(IsClassTemplate(src, "A<T,P,bool (*)(int, int),int N>"));
            REQUIRE(IsClassTemplate(src, "A<T,P=T,CMP=nullptr,int N=3>"));
            REQUIRE(IsClassTemplate(src, "B<T,C<>>"));

            REQUIRE(IsField<Public>(
                src, "A<T,P=T,CMP=nullptr,int N=3>", "value", "T"));
            REQUIRE(IsField<Public>(
                src, "A<T,P=T,CMP=nullptr,int N=3>", "pointer", "T *"));
            REQUIRE(IsField<Public>(
                src, "A<T,P=T,CMP=nullptr,int N=3>", "reference", "T &"));
            REQUIRE(IsField<Public>(src, "A<T,P=T,CMP=nullptr,int N=3>",
                "values", "std::vector<P>"));
            REQUIRE(IsField<Public>(src, "A<T,P=T,CMP=nullptr,int N=3>", "ints",
                "std::array<int,N>"));
            // TODO: add option to resolve using declared types
            // REQUIRE(IsField(src, Public("bool (*)(int, int) comparator"));
            REQUIRE(IsField<Public>(
                src, "A<T,P=T,CMP=nullptr,int N=3>", "comparator", "CMP"));

            REQUIRE(IsClassTemplate(src, "E::nested_template<ET>"));
            REQUIRE(IsClassTemplate(src, "E::nested_template<char>"));
            REQUIRE(IsInstantiation(
                src, "E::nested_template<ET>", "E::nested_template<char>"));
        },
        [](const plantuml_t &src) {
            REQUIRE(!IsClass(src, "E::nested_template"));
        });
}