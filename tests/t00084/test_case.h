/**
 * tests/t00084/test_case.h
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

TEST_CASE("t00084")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00084", "t00084_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsObjCProtocol(src, "PAdd"));
        REQUIRE(IsObjCProtocol(src, "PSub"));
        REQUIRE(IsObjCInterface(src, "CULogger"));
        REQUIRE(IsObjCCategory(src, "MatrixOps"));
        REQUIRE(IsClass(src, "CULogger"));

        REQUIRE(IsObjCInterface(src, "CUArithmetic"));
        REQUIRE(IsObjCInterface(src, "CUIntArithmetic"));
        REQUIRE(IsObjCInterface(src, "CUMatrixArithmetic"));

        REQUIRE(IsInstantiation(src, "PAdd", "CUArithmetic"));
        REQUIRE(IsInstantiation(src, "PSub", "CUArithmetic"));

        REQUIRE(IsBaseClass(src, "CUArithmetic", "CUIntArithmetic"));
        REQUIRE(IsBaseClass(src, "CUArithmetic", "CUMatrixArithmetic"));

        REQUIRE(IsMethod<Public>(src, "PAdd", "add"));
        REQUIRE(IsMethod<Public>(src, "PSub", "subtract"));
    });
}