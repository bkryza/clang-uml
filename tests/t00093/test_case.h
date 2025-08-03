/**
 * tests/t00093/test_case.h
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

TEST_CASE("t00093")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00093", "t00093_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsAbstractClass(src, "Renderer"));
        REQUIRE(IsMethod<Public, Abstract>(src, "Renderer", "render_circle",
            "void", "double x, double y, double radius"));
        REQUIRE(IsMethod<Public, Abstract>(src, "Renderer", "render_rectangle",
            "void", "double x, double y, double width, double height"));

        REQUIRE(IsClass(src, "VectorRenderer"));
        REQUIRE(IsClass(src, "RasterRenderer"));
        REQUIRE(IsBaseClass(src, "Renderer", "VectorRenderer"));
        REQUIRE(IsBaseClass(src, "Renderer", "RasterRenderer"));

        REQUIRE(IsMethod<Public>(src, "VectorRenderer", "render_circle", "void",
            "double x, double y, double radius"));
        REQUIRE(IsMethod<Public>(src, "VectorRenderer", "render_rectangle",
            "void", "double x, double y, double width, double height"));
        REQUIRE(IsMethod<Public>(
            src, "VectorRenderer", "set_stroke_width", "void", "double width"));
        REQUIRE(IsMethod<Public>(src, "VectorRenderer", "set_fill_color",
            "void", "const std::string & color"));

        REQUIRE(IsMethod<Public>(src, "RasterRenderer", "render_circle", "void",
            "double x, double y, double radius"));
        REQUIRE(IsMethod<Public>(src, "RasterRenderer", "render_rectangle",
            "void", "double x, double y, double width, double height"));
        REQUIRE(IsMethod<Public>(
            src, "RasterRenderer", "set_dpi", "void", "int dpi"));
        REQUIRE(IsMethod<Public>(src, "RasterRenderer", "set_anti_aliasing",
            "void", "bool enabled"));

        REQUIRE(IsAbstractClass(src, "Shape"));
        REQUIRE(IsMethod<Public, Abstract>(src, "Shape", "draw"));
        REQUIRE(IsMethod<Public, Abstract>(
            src, "Shape", "move", "void", "double dx, double dy"));

        REQUIRE(IsAssociation<Protected>(
            src, "Shape", "Renderer", "renderer_bridge_"));

        REQUIRE(IsClass(src, "Circle"));
        REQUIRE(IsClass(src, "Rectangle"));
        REQUIRE(IsBaseClass(src, "Shape", "Circle"));
        REQUIRE(IsBaseClass(src, "Shape", "Rectangle"));

        REQUIRE(IsMethod<Public>(src, "Circle", "draw"));
        REQUIRE(IsMethod<Public>(
            src, "Circle", "move", "void", "double dx, double dy"));
        REQUIRE(IsMethod<Public>(
            src, "Circle", "set_radius", "void", "double radius"));
        REQUIRE(IsMethod<Public, Const>(src, "Circle", "get_radius", "double"));

        REQUIRE(IsMethod<Public>(src, "Rectangle", "draw"));
        REQUIRE(IsMethod<Public>(
            src, "Rectangle", "move", "void", "double dx, double dy"));
        REQUIRE(IsMethod<Public>(src, "Rectangle", "set_dimensions", "void",
            "double width, double height"));
        REQUIRE(
            IsMethod<Public, Const>(src, "Rectangle", "get_width", "double"));
        REQUIRE(
            IsMethod<Public, Const>(src, "Rectangle", "get_height", "double"));

        REQUIRE(IsClass(src, "DrawingApplication"));
        REQUIRE(IsMethod<Public>(src, "DrawingApplication", "create_circle",
            "void", "double radius"));
        REQUIRE(IsMethod<Public>(src, "DrawingApplication", "create_rectangle",
            "void", "double width, double height"));
        REQUIRE(IsMethod<Public>(src, "DrawingApplication", "draw_all"));
        REQUIRE(IsMethod<Public>(src, "DrawingApplication", "move_all", "void",
            "double dx, double dy"));

        REQUIRE(IsAssociation<Private>(
            src, "DrawingApplication", "Renderer", "renderer_"));
        REQUIRE(IsAggregation<Private>(
            src, "DrawingApplication", "Shape", "shapes_"));
    });
}