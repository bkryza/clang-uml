/**
 * tests/t00102/test_case.h
 *
 * Copyright (c) 2021-2026 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t00102")
{
    using namespace clanguml::test;

    auto [class_config, class_db, class_diagram, class_model] =
        CHECK_CLASS_MODEL("t00102", "t00102_class");

    CHECK_CLASS_DIAGRAM(*class_config, class_diagram, *class_model,
        [](const auto &src) { REQUIRE(IsClassTemplate(src, "wrapper<C,T>")); });

    auto [sequence_config, sequence_db, sequence_diagram, sequence_model] =
        CHECK_SEQUENCE_MODEL("t00102", "t00102_sequence");

    CHECK_SEQUENCE_DIAGRAM(
        *sequence_config, sequence_diagram, *sequence_model,
        [](const auto &) { }, [](const json_t &) { });
}