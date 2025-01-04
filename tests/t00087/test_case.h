/**
 * tests/t00087/test_case.h
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t00087")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00087", "t00087_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "FooClass"));
        REQUIRE(!IsEnum(src, "FooEnum"));
        REQUIRE(IsClassTemplate(src, "Bar<T>"));

        if constexpr (!std::is_same_v<graphml_t, std::decay_t<decltype(src)>>) {
            REQUIRE(!IsMethod<Public, Const>(src, "FooClass", "getFoo"));
            REQUIRE(!IsMethod<Public>(src, "FooClass", "setFoo", "int"));
            REQUIRE(!IsMethod<Public>(
                src, "FooClass", "makeFooClass_static", "void"));
        }

        REQUIRE(IsMethod<Public>(src, "FooClass", "foo"));
        REQUIRE(IsMethod<Public>(src, "FooClass", "bar"));

        REQUIRE(IsField<Private>(src, "FooClass", "foo_", "int"));

        if constexpr (!std::is_same_v<graphml_t, std::decay_t<decltype(src)>>) {
            REQUIRE(!IsField<Private>(src, "FooClass", "pImpl_", "void *"));
            REQUIRE(
                !IsField<Public>(src, "FooClass", "fooCount_static", "int"));
        }
    });
}