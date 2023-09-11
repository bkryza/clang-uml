/**
 * tests/t00066/test_case.h
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

TEST_CASE("t00066", "[test-case][class]")
{
    auto [config, db] = load_config("t00066");

    auto diagram = config.diagrams["t00066_class"];

    REQUIRE(diagram->name == "t00066_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00066_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsClass(_A("A")));

        REQUIRE_THAT(src, !IsDependency(_A("A"), _A("A")));

        REQUIRE_THAT(src, (IsMethod<Public, Default>("A")));
        REQUIRE_THAT(src, (IsMethod<Public, Default>("A", "void", "A &&")));
        REQUIRE_THAT(
            src, (IsMethod<Public, Deleted>("A", "void", "const A &")));

        REQUIRE_THAT(src, (IsMethod<Public, Default>("~A")));

        REQUIRE_THAT(src, (IsMethod<Public>("basic_method")));
        REQUIRE_THAT(src, (IsMethod<Public, Static>("static_method", "int")));
        REQUIRE_THAT(src, (IsMethod<Public, Const>("const_method")));
        REQUIRE_THAT(
            src, (IsMethod<Public>("default_int", "int", "int i = 12")));
        REQUIRE_THAT(src,
            (IsMethod<Public>("default_string", "std::string",
                "int i, std::string s = \"abc\"")));

        REQUIRE_THAT(src, (IsMethod<Public, Const>("size", "std::size_t")));

        REQUIRE_THAT(src, (IsMethod<Protected>("protected_method")));
        REQUIRE_THAT(src, (IsMethod<Private>("private_method")));
        REQUIRE_THAT(src, (IsField<Public>("public_member", "int")));
        REQUIRE_THAT(src, (IsField<Protected>("protected_member", "int")));
        REQUIRE_THAT(src, (IsField<Private>("private_member", "int")));
        REQUIRE_THAT(src,
            (IsField<Public, Static>("auto_member", "const unsigned long")));

        REQUIRE_THAT(src, (IsField<Private>("a_", "int")));
        REQUIRE_THAT(src, (IsField<Private>("b_", "int")));
        REQUIRE_THAT(src, (IsField<Private>("c_", "int")));

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
        using mermaid::IsField;
        using mermaid::IsMethod;

        REQUIRE_THAT(src, IsClass(_A("A")));

        REQUIRE_THAT(src, !IsDependency(_A("A"), _A("A")));

        REQUIRE_THAT(src, (IsMethod<Public, Default>("A")));
        REQUIRE_THAT(src, (IsMethod<Public, Default>("A", "void", "A &&")));
        REQUIRE_THAT(
            src, (IsMethod<Public, Deleted>("A", "void", "const A &")));

        REQUIRE_THAT(src, (IsMethod<Public, Default>("~A")));

        REQUIRE_THAT(src, (IsMethod<Public>("basic_method")));
        REQUIRE_THAT(src, (IsMethod<Public, Static>("static_method", "int")));
        REQUIRE_THAT(src, (IsMethod<Public, Const>("const_method")));
        REQUIRE_THAT(
            src, (IsMethod<Public>("default_int", "int", "int i = 12")));
        REQUIRE_THAT(src,
            (IsMethod<Public>("default_string", "std::string",
                "int i, std::string s = \"abc\"")));

        REQUIRE_THAT(src, (IsMethod<Public, Const>("size", "std::size_t")));

        REQUIRE_THAT(src, (IsMethod<Protected>("protected_method")));
        REQUIRE_THAT(src, (IsMethod<Private>("private_method")));
        REQUIRE_THAT(src, (IsField<Public>("public_member", "int")));
        REQUIRE_THAT(src, (IsField<Protected>("protected_member", "int")));
        REQUIRE_THAT(src, (IsField<Private>("private_member", "int")));
        REQUIRE_THAT(src,
            (IsField<Public, Static>("auto_member", "const unsigned long")));

        REQUIRE_THAT(src, (IsField<Private>("a_", "int")));
        REQUIRE_THAT(src, (IsField<Private>("b_", "int")));
        REQUIRE_THAT(src, (IsField<Private>("c_", "int")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}