/**
 * tests/t00094/test_case.h
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

TEST_CASE("t00094")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00094", "t00094_class");

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

        REQUIRE(IsAbstractClass(src, "ComputerBuilder"));
        REQUIRE(
            IsMethod<Public, Abstract>(src, "ComputerBuilder", "build_cpu"));
        REQUIRE(
            IsMethod<Public, Abstract>(src, "ComputerBuilder", "build_memory"));
        REQUIRE(IsMethod<Public, Abstract>(
            src, "ComputerBuilder", "build_storage"));
        REQUIRE(IsMethod<Public, Abstract>(
            src, "ComputerBuilder", "build_graphics_card"));
        REQUIRE(IsMethod<Public, Abstract>(
            src, "ComputerBuilder", "build_motherboard"));
        REQUIRE(
            IsMethod<Public, Abstract>(src, "ComputerBuilder", "build_ports"));
        REQUIRE(IsMethod<Public, Abstract>(
            src, "ComputerBuilder", "build_power_supply"));
        REQUIRE(
            IsMethod<Public, Abstract>(src, "ComputerBuilder", "build_case"));
        REQUIRE(IsMethod<Public, Abstract>(src, "ComputerBuilder",
            "get_computer", "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public, Abstract>(src, "ComputerBuilder", "reset"));

        REQUIRE(IsAggregation<Protected>(
            src, "ComputerBuilder", "Computer", "computer_"));

        REQUIRE(IsClass(src, "GamingComputerBuilder"));
        REQUIRE(IsClass(src, "OfficeComputerBuilder"));
        REQUIRE(IsBaseClass(src, "ComputerBuilder", "GamingComputerBuilder"));
        REQUIRE(IsBaseClass(src, "ComputerBuilder", "OfficeComputerBuilder"));

        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_cpu"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_memory"));
        REQUIRE(
            IsMethod<Public>(src, "GamingComputerBuilder", "build_storage"));
        REQUIRE(IsMethod<Public>(
            src, "GamingComputerBuilder", "build_graphics_card"));
        REQUIRE(IsMethod<Public>(
            src, "GamingComputerBuilder", "build_motherboard"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_ports"));
        REQUIRE(IsMethod<Public>(
            src, "GamingComputerBuilder", "build_power_supply"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "build_case"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "get_computer",
            "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "GamingComputerBuilder", "reset"));

        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_cpu"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_memory"));
        REQUIRE(
            IsMethod<Public>(src, "OfficeComputerBuilder", "build_storage"));
        REQUIRE(IsMethod<Public>(
            src, "OfficeComputerBuilder", "build_graphics_card"));
        REQUIRE(IsMethod<Public>(
            src, "OfficeComputerBuilder", "build_motherboard"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_ports"));
        REQUIRE(IsMethod<Public>(
            src, "OfficeComputerBuilder", "build_power_supply"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "build_case"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "get_computer",
            "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "OfficeComputerBuilder", "reset"));

        REQUIRE(IsClass(src, "ComputerDirector"));
        REQUIRE(IsMethod<Public>(src, "ComputerDirector", "set_builder", "void",
            "std::unique_ptr<ComputerBuilder> builder"));
        REQUIRE(IsMethod<Public>(src, "ComputerDirector",
            "construct_basic_computer", "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "ComputerDirector",
            "construct_full_computer", "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "ComputerDirector",
            "construct_custom_computer", "std::unique_ptr<Computer>"));

        REQUIRE(IsAggregation<Private>(
            src, "ComputerDirector", "ComputerBuilder", "builder_"));

        REQUIRE(IsClass(src, "ComputerStore"));
        REQUIRE(IsMethod<Public>(src, "ComputerStore", "order_gaming_computer",
            "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "ComputerStore", "order_office_computer",
            "std::unique_ptr<Computer>"));
        REQUIRE(IsMethod<Public>(src, "ComputerStore", "order_custom_computer",
            "std::unique_ptr<Computer>",
            "std::unique_ptr<ComputerBuilder> builder"));

        REQUIRE(IsDependency(src, "ComputerStore", "Computer"));
        REQUIRE(IsDependency(src, "ComputerStore", "ComputerBuilder"));
        REQUIRE(IsDependency(src, "ComputerDirector", "Computer"));
    });
}