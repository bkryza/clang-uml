/**
 * tests/t00033/test_case.h
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

TEST_CASE("t00033")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00033");

    auto diagram = config.diagrams["t00033_class"];

    REQUIRE(diagram->name == "t00033_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00033_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClassTemplate(src, "A<T>"));
        REQUIRE(IsClassTemplate(src, "B<T>"));
        REQUIRE(IsClassTemplate(src, "C<T>"));
        REQUIRE(IsClass(src, "D"));
        REQUIRE(IsClass(src, "R"));

        REQUIRE(IsDependency(
            src, "A<B<std::unique_ptr<C<D>>>>", "B<std::unique_ptr<C<D>>>"));
        REQUIRE(IsDependency(src, "B<std::unique_ptr<C<D>>>", "C<D>"));
        REQUIRE(IsDependency(src, "C<D>", "D"));

        REQUIRE(IsInstantiation(src, "C<T>", "C<D>", "up"));
        REQUIRE(IsInstantiation(src, "B<T>", "B<std::unique_ptr<C<D>>>", "up"));
        REQUIRE(
            IsInstantiation(src, "A<T>", "A<B<std::unique_ptr<C<D>>>>", "up"));
    });
}