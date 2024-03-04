/**
 * tests/t90001/test_case.cc
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

TEST_CASE("t90001", "[test-case][config]")
{
    using clanguml::error::empty_diagram_error;

    auto [config, db] = load_config("t90001");

    {
        auto diagram = config.diagrams["t90001_class"];

        auto model = generate_class_diagram(*db, diagram);

        REQUIRE(model->is_empty());

        REQUIRE(model->name() == "t90001_class");

        REQUIRE_THROWS_AS(
            generate_class_puml(diagram, *model), empty_diagram_error);
        REQUIRE_THROWS_AS(
            generate_class_json(diagram, *model), empty_diagram_error);
        REQUIRE_THROWS_AS(
            generate_class_mermaid(diagram, *model), empty_diagram_error);
    }

    {
        auto diagram = config.diagrams["t90001_sequence"];

        auto model = generate_sequence_diagram(*db, diagram);

        REQUIRE(model->is_empty());

        REQUIRE(model->name() == "t90001_sequence");

        REQUIRE_THROWS_AS(
            generate_sequence_puml(diagram, *model), empty_diagram_error);
        REQUIRE_THROWS_AS(
            generate_sequence_json(diagram, *model), empty_diagram_error);
        REQUIRE_THROWS_AS(
            generate_sequence_mermaid(diagram, *model), empty_diagram_error);
    }

    {
        auto diagram = config.diagrams["t90001_package"];

        auto model = generate_package_diagram(*db, diagram);

        REQUIRE(model->is_empty());

        REQUIRE(model->name() == "t90001_package");

        REQUIRE_THROWS_AS(
            generate_package_puml(diagram, *model), empty_diagram_error);
        REQUIRE_THROWS_AS(
            generate_package_json(diagram, *model), empty_diagram_error);
        REQUIRE_THROWS_AS(
            generate_package_mermaid(diagram, *model), empty_diagram_error);
    }

    {
        auto diagram = config.diagrams["t90001_include"];

        auto model = generate_include_diagram(*db, diagram);

        REQUIRE(model->is_empty());

        REQUIRE(model->name() == "t90001_include");

        REQUIRE_THROWS_AS(
            generate_include_puml(diagram, *model), empty_diagram_error);
        REQUIRE_THROWS_AS(
            generate_include_json(diagram, *model), empty_diagram_error);
        REQUIRE_THROWS_AS(
            generate_include_mermaid(diagram, *model), empty_diagram_error);
    }
}
