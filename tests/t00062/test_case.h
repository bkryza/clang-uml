/**
 * tests/t00062/test_case.h
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

TEST_CASE("t00062")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00062", "t00062_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(!src.contains("type-parameter-"));

        REQUIRE(IsClassTemplate(src, "A<T>"));
        REQUIRE(IsClassTemplate(src, "A<U &>"));
        REQUIRE(IsClassTemplate(src, "A<U &&>"));
        REQUIRE(IsClassTemplate(src, "A<U const&>"));
        REQUIRE(IsClassTemplate(src, "A<U * *>"));
        REQUIRE(IsClassTemplate(src, "A<M C::*>"));
        REQUIRE(IsClassTemplate(src, "A<M C::* &&>"));
        REQUIRE(IsClassTemplate(src, "A<M (C::*)(Arg)>"));
        REQUIRE(IsClassTemplate(src, "A<int (C::*)(bool)>"));
        REQUIRE(IsClassTemplate(src, "A<M (C::*)(Arg) &&>"));
        REQUIRE(IsClassTemplate(src, "A<M (C::*)(Arg1,Arg2,Arg3)>"));
        REQUIRE(IsClassTemplate(src, "A<float (C::*)(int) &&>"));

        REQUIRE(IsClassTemplate(src, "A<char[N]>"));
        REQUIRE(IsClassTemplate(src, "A<char[1000]>"));

        REQUIRE(IsClassTemplate(src, "A<U(...)>"));
        REQUIRE(IsClassTemplate(src, "A<C<T>>"));
        REQUIRE(IsClassTemplate(src, "A<C<T,Args...>>"));

        REQUIRE(IsField<Public>(src, "A<U &>", "u", "U &"));
        REQUIRE(IsField<Public>(src, "A<U * * const*>", "u", "U ***"));
        REQUIRE(IsField<Public>(src, "A<U &&>", "u", "U &&"));
        REQUIRE(IsField<Public>(src, "A<U const&>", "u", "const U &"));
        REQUIRE(IsField<Public>(src, "A<M C::*>", "m", "M C::*"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<U &>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<U &&>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<M C::*>"));
        REQUIRE(IsInstantiation(src, "A<U &&>", "A<M C::* &&>"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<M (C::*)(Arg)>"));
        REQUIRE(
            IsInstantiation(src, "A<M (C::*)(Arg)>", "A<int (C::*)(bool)>"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<char[N]>"));
        REQUIRE(IsInstantiation(src, "A<char[N]>", "A<char[1000]>"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<U(...)>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<U(...)>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<C<T>>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<C<T,Args...>>"));
    });
}