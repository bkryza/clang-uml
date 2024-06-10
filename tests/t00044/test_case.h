/**
 * tests/t00044/test_case.h
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

TEST_CASE("t00044")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00044", "t00044_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(!src.contains("type-parameter-"));

        REQUIRE(IsClassTemplate(src, "sink<T>"));
        REQUIRE(IsClassTemplate(src, "signal_handler<T,A>"));

        REQUIRE(IsClassTemplate(src, "signal_handler<Ret(Args...),A>"));
        REQUIRE(IsClassTemplate(src, "signal_handler<void(int),bool>"));

        REQUIRE(IsClassTemplate(src, "sink<signal_handler<Ret(Args...),A>>"));

        REQUIRE(IsInstantiation(
            src, "sink<T>", "sink<signal_handler<Ret(Args...),A>>"));

        REQUIRE(IsInstantiation(src, "sink<signal_handler<Ret(Args...),A>>",
            "sink<signal_handler<void(int),bool>>"));

        REQUIRE(IsClassTemplate(src, "signal_handler<T,A>"));
        REQUIRE(IsInstantiation(
            src, "signal_handler<T,A>", "signal_handler<Ret(Args...),A>"));

        REQUIRE(IsInstantiation(src, "signal_handler<Ret(Args...),A>",
            "signal_handler<void(int),bool>"));
    });
}