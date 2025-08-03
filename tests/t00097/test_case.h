/**
 * tests/t00097/test_case.h
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

TEST_CASE("t00097")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00097", "t00097_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(HasTitle(src, "Decorator design pattern"));

        REQUIRE(IsAbstractClass(src, "Component"));
        REQUIRE(IsMethod<Public, Abstract, Const>(
            src, "Component", "operation", "std::string"));

        REQUIRE(IsClass(src, "ConcreteComponent"));
        REQUIRE(IsBaseClass(src, "Component", "ConcreteComponent"));
        REQUIRE(IsMethod<Public, Const>(
            src, "ConcreteComponent", "operation", "std::string"));

        REQUIRE(IsClass(src, "Decorator"));
        REQUIRE(IsBaseClass(src, "Component", "Decorator"));
        REQUIRE(IsAssociation<Protected>(
            src, "Decorator", "Component", "component_"));
        REQUIRE(IsMethod<Public, Const>(
            src, "Decorator", "operation", "std::string"));

        REQUIRE(IsClass(src, "ConcreteDecoratorA"));
        REQUIRE(IsBaseClass(src, "Decorator", "ConcreteDecoratorA"));
        REQUIRE(IsMethod<Public, Const>(
            src, "ConcreteDecoratorA", "operation", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "ConcreteDecoratorA", "added_behavior", "std::string"));

        REQUIRE(IsClass(src, "ConcreteDecoratorB"));
        REQUIRE(IsBaseClass(src, "Decorator", "ConcreteDecoratorB"));
        REQUIRE(IsMethod<Public, Const>(
            src, "ConcreteDecoratorB", "operation", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "ConcreteDecoratorB", "additional_method"));

        REQUIRE(IsClass(src, "Client"));
        REQUIRE(
            IsAssociation<Private>(src, "Client", "Component", "component_"));
        REQUIRE(IsMethod<Public>(src, "Client", "execute", "std::string"));
    });
}