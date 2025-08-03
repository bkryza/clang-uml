/**
 * tests/t00098/test_case.h
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

TEST_CASE("t00098")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00098", "t00098_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(HasTitle(
            src, "Decorator design pattern with templates and concepts"));

        REQUIRE(IsConcept(src, "Component<T>"));

        REQUIRE(IsClass(src, "ConcreteComponent"));
        REQUIRE(IsClassTemplate(src, "ConcreteDecoratorA<Component Inner>"));
        REQUIRE(IsClassTemplate(src, "ConcreteDecoratorB<Component Inner>"));
        REQUIRE(IsClassTemplate(src, "Client<Component C>"));

        REQUIRE(IsField<Private>(
            src, "ConcreteDecoratorA<Component Inner>", "inner_", "Inner"));
        REQUIRE(IsField<Private>(
            src, "ConcreteDecoratorB<Component Inner>", "inner_", "Inner"));
        REQUIRE(
            IsField<Private>(src, "Client<Component C>", "component_", "C"));

        REQUIRE(IsMethod<Public, Const>(
            src, "ConcreteComponent", "operation", "std::string"));
        REQUIRE(IsMethod<Public, Const>(src,
            "ConcreteDecoratorA<Component Inner>", "operation", "std::string"));
        REQUIRE(
            IsMethod<Public, Const>(src, "ConcreteDecoratorA<Component Inner>",
                "added_behavior", "std::string"));
        REQUIRE(IsMethod<Public, Const>(src,
            "ConcreteDecoratorB<Component Inner>", "operation", "std::string"));
        REQUIRE(
            IsMethod<Public, Const>(src, "ConcreteDecoratorB<Component Inner>",
                "additional_method", "void"));
        REQUIRE(IsMethod<Public>(
            src, "Client<Component C>", "execute", "std::string"));

        REQUIRE(IsInstantiation(src, "ConcreteDecoratorA<Component Inner>",
            "ConcreteDecoratorA<ConcreteComponent>"));
        REQUIRE(IsInstantiation(src, "ConcreteDecoratorB<Component Inner>",
            "ConcreteDecoratorB<ConcreteDecoratorA<ConcreteComponent>>"));
        REQUIRE(IsInstantiation(src, "Client<Component C>",
            "Client<ConcreteDecoratorB<ConcreteDecoratorA<ConcreteComponent>>"
            ">"));

        REQUIRE(IsClass(src, "R"));
        REQUIRE(IsField<Public>(src, "R", "client", "BAClient"));
    });
}