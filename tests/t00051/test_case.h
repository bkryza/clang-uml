/**
 * tests/t00051/test_case.h
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

TEST_CASE("t00051")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00051", "t00051_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsInnerClass(src, "A", "A::custom_thread1"));
        REQUIRE(IsInnerClass(src, "A", "A::custom_thread2"));

        REQUIRE(IsMethod<Public>(src, "A::custom_thread1",
            "custom_thread1<Function,Args...>", "void",
            "Function && f, Args &&... args"));
        REQUIRE(IsMethod<Public>(src, "A::custom_thread2", "thread", "void",
            "(lambda at t00051.cc:59:27) &&"));
        REQUIRE(IsMethod<Private>(src, "A", "start_thread3",
            "B<(lambda at t00051.cc:43:18),(lambda at "
            "t00051.cc:43:27)>"));
        REQUIRE(IsMethod<Private>(
            src, "A", "get_function", "(lambda at t00051.cc:48:16)"));

        REQUIRE(IsClassTemplate(src, "B<F,FF=F>"));
        REQUIRE(IsMethod<Public>(src, "B<F,FF=F>", "f", "void"));
        REQUIRE(IsMethod<Public>(src, "B<F,FF=F>", "ff", "void"));

        REQUIRE(IsClassTemplate(
            src, "B<(lambda at t00051.cc:43:18),(lambda at t00051.cc:43:27)>"));

        REQUIRE(IsInstantiation(src, "B<F,FF=F>",
            "B<(lambda at t00051.cc:43:18),(lambda at t00051.cc:43:27)>"));

        REQUIRE(IsDependency(src, "A",
            "B<(lambda at t00051.cc:43:18),(lambda at t00051.cc:43:27)>"));
    });
}