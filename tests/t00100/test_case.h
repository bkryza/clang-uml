/**
 * tests/t00100/test_case.h
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

TEST_CASE("t00100")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00100", "t00100_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "Computer"));
        REQUIRE(IsMethod<Public>(
            src, "Computer", "set_cpu", "void", "const std::string & cpu"));
        REQUIRE(IsMethod<Public>(
            src, "Computer", "set_memory", "void", "int memory_gb"));
        REQUIRE(IsMethod<Public>(src, "Computer", "set_storage", "void",
            "const std::string & storage"));
        REQUIRE(IsMethod<Public>(src, "Computer", "set_graphics_card", "void",
            "const std::string & gpu"));
        REQUIRE(IsMethod<Public>(src, "Computer", "set_motherboard", "void",
            "const std::string & motherboard"));
        REQUIRE(IsMethod<Public>(
            src, "Computer", "add_port", "void", "const std::string & port"));
        REQUIRE(IsMethod<Public>(
            src, "Computer", "set_power_supply", "void", "int watts"));
        REQUIRE(IsMethod<Public>(src, "Computer", "set_case_type", "void",
            "const std::string & case_type"));
        REQUIRE(IsMethod<Public, Const>(
            src, "Computer", "get_cpu", "const std::string &"));
        REQUIRE(IsMethod<Public, Const>(src, "Computer", "get_memory", "int"));
        REQUIRE(IsMethod<Public, Const>(src, "Computer", "display_specs"));

        REQUIRE(IsConcept(src, "ComputerBuilderConcept<T>"));
        REQUIRE(IsConceptRequirement(src, "ComputerBuilderConcept<T>",
            "{t.build_cpu(computer)} -> std::same_as<void>"));
        REQUIRE(IsConceptRequirement(src, "ComputerBuilderConcept<T>",
            "{t.build_memory(computer)} -> std::same_as<void>"));
        REQUIRE(IsConceptRequirement(src, "ComputerBuilderConcept<T>",
            "{t.build_storage(computer)} -> std::same_as<void>"));
        REQUIRE(IsConceptRequirement(src, "ComputerBuilderConcept<T>",
            "{t.build_graphics_card(computer)} -> std::same_as<void>"));
        REQUIRE(IsConceptRequirement(src, "ComputerBuilderConcept<T>",
            "{t.build_motherboard(computer)} -> std::same_as<void>"));
        REQUIRE(IsConceptRequirement(src, "ComputerBuilderConcept<T>",
            "{t.build_ports(computer)} -> std::same_as<void>"));
        REQUIRE(IsConceptRequirement(src, "ComputerBuilderConcept<T>",
            "{t.build_power_supply(computer)} -> std::same_as<void>"));
        REQUIRE(IsConceptRequirement(src, "ComputerBuilderConcept<T>",
            "{t.build_case(computer)} -> std::same_as<void>"));

        REQUIRE(IsClassTemplate(src, "ComputerBuilderBase<Derived>"));
        REQUIRE(IsMethod<Public>(src, "ComputerBuilderBase<Derived>",
            "get_computer", "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "ComputerBuilderBase<Derived>", "reset"));
        REQUIRE(IsMethod<Public>(src, "ComputerBuilderBase<Derived>",
            "get_computer_ref", "Computer &"));

        REQUIRE(IsAggregation<Protected>(
            src, "ComputerBuilderBase<Derived>", "Computer", "computer_"));

        REQUIRE(IsClass(src, "GamingComputerBuilder"));
        REQUIRE(IsClass(src, "OfficeComputerBuilder"));
        REQUIRE(IsBaseClass(src, "ComputerBuilderBase<GamingComputerBuilder>",
            "GamingComputerBuilder"));
        REQUIRE(IsBaseClass(src, "ComputerBuilderBase<OfficeComputerBuilder>",
            "OfficeComputerBuilder"));

        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_cpu",
            "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_memory",
            "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_storage",
            "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder",
            "build_graphics_card", "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder",
            "build_motherboard", "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_ports",
            "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder",
            "build_power_supply", "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_case",
            "void", "Computer & computer"));

        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_cpu",
            "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_memory",
            "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_storage",
            "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder",
            "build_graphics_card", "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder",
            "build_motherboard", "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_ports",
            "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder",
            "build_power_supply", "void", "Computer & computer"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_case",
            "void", "Computer & computer"));

        REQUIRE(IsClassTemplate(
            src, "ComputerDirector<ComputerBuilderConcept BuilderType>"));
        REQUIRE(IsMethod<Public>(src,
            "ComputerDirector<ComputerBuilderConcept BuilderType>",
            "construct_basic_computer", "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src,
            "ComputerDirector<ComputerBuilderConcept BuilderType>",
            "construct_full_computer", "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src,
            "ComputerDirector<ComputerBuilderConcept BuilderType>",
            "construct_custom_computer", "std::unique_ptr<Computer>"));

        REQUIRE(IsClass(src, "ComputerStore"));
        REQUIRE(IsMethod<Public>(src, "ComputerStore", "order_gaming_computer",
            "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "ComputerStore", "order_office_computer",
            "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "ComputerStore",
            "order_custom_computer<ComputerBuilderConcept BuilderType>",
            "std::unique_ptr<Computer>", "BuilderType & builder"));

        REQUIRE(IsDependency(src, "ComputerStore", "Computer"));
        REQUIRE(IsDependency(src, "GamingComputerBuilder", "Computer"));
        REQUIRE(IsDependency(src, "OfficeComputerBuilder", "Computer"));
    });
}