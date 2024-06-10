/**
 * tests/t00025/test_case.h
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

TEST_CASE("t00025")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00025", "t00025_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "Target1"));
        REQUIRE(IsClass(src, "Target2"));
        REQUIRE(IsClassTemplate(src, "Proxy<T>"));
        REQUIRE(IsDependency(src, "Proxy<Target1>", "Target1"));
        REQUIRE(IsDependency(src, "Proxy<Target2>", "Target2"));

        REQUIRE(IsInstantiation(src, "Proxy<T>", "Proxy<Target1>"));
        REQUIRE(IsInstantiation(src, "Proxy<T>", "Proxy<Target2>"));
        REQUIRE(IsAggregation<Public>(
            src, "ProxyHolder", "Proxy<Target1>", "proxy1"));
        REQUIRE(IsAggregation<Public>(
            src, "ProxyHolder", "Proxy<Target2>", "proxy2"));
        REQUIRE(
            !IsAggregation<Public>(src, "ProxyHolder", "Target1", "proxy1"));
        REQUIRE(
            !IsAggregation<Public>(src, "ProxyHolder", "Target2", "proxy2"));
    });
}