/**
 * tests/t00064/test_case.h
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

TEST_CASE("t00064")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00064", "t00064_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(!src.contains("type-parameter-"));

        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClass(src, "C"));
        REQUIRE(IsClass(src, "R"));

        REQUIRE(IsClassTemplate(src, "type_list<Ts...>"));
        REQUIRE(IsClassTemplate(src, "type_list<Ret(Arg &&),Ts...>"));
        REQUIRE(IsClassTemplate(src, "type_list<T const,Ts...>"));

        REQUIRE(IsClassTemplate(src, "head<typename>"));
        REQUIRE(IsClassTemplate(src, "head<type_list<Head,Tail...>>"));
        REQUIRE(IsClassTemplate(src, "type_group_pair<typename,typename>"));
        REQUIRE(IsClassTemplate(
            src, "type_group_pair<type_list<First...>,type_list<Second...>>"));
        REQUIRE(IsClassTemplate(
            src, "type_group_pair<type_list<float,double>,type_list<A,B,C>>"));

        REQUIRE(IsClassTemplate(src, "optional_ref<T>"));

        REQUIRE(IsClassTemplate(src,
            "type_group_pair_it<It,type_list<First...>,type_list<Second...>>"));
        REQUIRE(IsMethod<Public>(src,
            "type_group_pair_it<It,type_list<First...>,type_list<Second...>>",
            "get", "ref_t", "unsigned int i"));
#if LLVM_VERSION_MAJOR < 16
        REQUIRE(IsMethod<Public>(src,
            "type_group_pair_it<It,type_list<First...>,type_list<Second...>>",
            "getp", "value_type const*", "unsigned int i"));
        REQUIRE(IsMethod<Public, Constexpr>(src,
            "type_group_pair_it<It,type_list<First...>,type_list<Second...>>",
            "find", "unsigned int", "value_type const& v"));
#else
        REQUIRE(
            IsMethod<Public>(src,
                "type_group_pair_it<It,type_list<First...>,type_list<Second...>>",
                "getp", "const value_type *", "unsigned int i"));
        REQUIRE(
            IsMethod<Public, Constexpr>(src,"type_group_pair_it<It,type_list<First...>,type_list<Second...>>",
                "find", "unsigned int", "const value_type & v"));
#endif
    });
}