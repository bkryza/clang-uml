/**
 * tests/t90001/test_case.h
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

TEST_CASE("t90001")
{
    using namespace clanguml::test;
    using clanguml::error::empty_diagram_error;

    auto [cfg, db] = load_config("t90001");

    const auto &config = *cfg;

    {
        auto diagram = config.diagrams.at("t90001_class");

        auto model = generate_class_diagram(*db, diagram);

        REQUIRE(model->is_empty());

        REQUIRE(model->name() == "t90001_class");

        REQUIRE_THROWS_AS(render_class_diagram<plantuml_t>(diagram, *model),
            empty_diagram_error);
        REQUIRE_THROWS_AS(render_class_diagram<mermaid_t>(diagram, *model),
            empty_diagram_error);
        REQUIRE_THROWS_AS(
            render_class_diagram<json_t>(diagram, *model), empty_diagram_error);
    }

    {
        auto diagram = config.diagrams.at("t90001_sequence");

        auto model = generate_sequence_diagram(*db, diagram);

        REQUIRE(model->is_empty());

        REQUIRE(model->name() == "t90001_sequence");

        REQUIRE_THROWS_AS(render_sequence_diagram<plantuml_t>(diagram, *model),
            empty_diagram_error);
        REQUIRE_THROWS_AS(render_sequence_diagram<mermaid_t>(diagram, *model),
            empty_diagram_error);
        REQUIRE_THROWS_AS(render_sequence_diagram<json_t>(diagram, *model),
            empty_diagram_error);
    }

    {
        auto diagram = config.diagrams.at("t90001_package");

        auto model = generate_package_diagram(*db, diagram);

        REQUIRE(model->is_empty());

        REQUIRE(model->name() == "t90001_package");

        REQUIRE_THROWS_AS(render_package_diagram<plantuml_t>(diagram, *model),
            empty_diagram_error);
        REQUIRE_THROWS_AS(render_package_diagram<mermaid_t>(diagram, *model),
            empty_diagram_error);
        REQUIRE_THROWS_AS(render_package_diagram<json_t>(diagram, *model),
            empty_diagram_error);
    }

    {
        auto diagram = config.diagrams.at("t90001_include");

        auto model = generate_include_diagram(*db, diagram);

        REQUIRE(model->is_empty());

        REQUIRE(model->name() == "t90001_include");

        REQUIRE_THROWS_AS(render_include_diagram<plantuml_t>(diagram, *model),
            empty_diagram_error);
        REQUIRE_THROWS_AS(render_include_diagram<mermaid_t>(diagram, *model),
            empty_diagram_error);
        REQUIRE_THROWS_AS(render_include_diagram<json_t>(diagram, *model),
            empty_diagram_error);
    }
}
