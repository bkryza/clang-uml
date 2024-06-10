/**
 * tests/t00003/test_case.h
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

TEST_CASE("t00003")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00003", "t00003_class");

    REQUIRE(diagram->include().namespaces.size() == 1);
    REQUIRE(diagram->exclude().namespaces.size() == 0);

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));

        REQUIRE(!IsDependency(src, "A", "A"));

        REQUIRE(IsMethod<Public, Default>(src, "A", "A"));
        REQUIRE(IsMethod<Public, Default>(src, "A", "A", "void", "A &&"));
        REQUIRE(IsMethod<Public, Deleted>(src, "A", "A", "void", "const A &"));

        REQUIRE(IsMethod<Public, Default>(src, "A", "~A"));

        REQUIRE(IsMethod<Public>(src, "A", "basic_method"));
        REQUIRE(IsMethod<Public, Static>(src, "A", "static_method", "int"));
        REQUIRE(IsMethod<Public, Const>(src, "A", "const_method"));
        REQUIRE(IsMethod<Public>(src, "A", "default_int", "int", "int i = 12"));
        REQUIRE(IsMethod<Public>(src, "A", "default_string", "std::string",
            "int i, std::string s = \"abc\""));

        REQUIRE(IsMethod<Public, Const, Constexpr>(
            src, "A", "size", "std::size_t"));

        REQUIRE(IsMethod<Protected>(src, "A", "protected_method"));
        REQUIRE(IsMethod<Private>(src, "A", "private_method"));
        REQUIRE(IsField<Public>(src, "A", "public_member", "int"));
        REQUIRE(IsField<Protected>(src, "A", "protected_member", "int"));
        REQUIRE(IsField<Private>(src, "A", "private_member", "int"));
        REQUIRE(IsField<Public, Static>(
            src, "A", "auto_member", "const unsigned long"));

        REQUIRE(IsField<Private>(src, "A", "a_", "int"));
        REQUIRE(IsField<Private>(src, "A", "b_", "int"));
        REQUIRE(IsField<Private>(src, "A", "c_", "int"));
    });
}