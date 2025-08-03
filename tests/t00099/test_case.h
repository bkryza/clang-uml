/**
 * tests/t00099/test_case.h
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

TEST_CASE("t00099")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00099", "t00099_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(
            HasTitle(src, "Bridge design pattern with static polymorphism"));

        REQUIRE(IsConcept(src, "Renderer<T>"));

        REQUIRE(IsClass(src, "VectorRenderer"));
        REQUIRE(IsClass(src, "RasterRenderer"));
        REQUIRE(IsAbstractClassTemplate(src, "Shape<Renderer R>"));
        REQUIRE(!IsClassTemplate(src, "Shape<R>"));

        REQUIRE(IsClassTemplate(src, "Circle<Renderer R>"));
        REQUIRE(IsClassTemplate(src, "Rectangle<Renderer R>"));
        REQUIRE(IsClassTemplate(src, "DrawingApplication<Renderer R>"));

        REQUIRE(
            IsField<Private>(src, "VectorRenderer", "stroke_width", "double"));
        REQUIRE(IsField<Private>(
            src, "VectorRenderer", "fill_color", "std::string"));
        REQUIRE(IsField<Private>(src, "RasterRenderer", "dpi", "int"));
        REQUIRE(
            IsField<Private>(src, "RasterRenderer", "anti_aliasing", "bool"));
        REQUIRE(IsField<Protected>(
            src, "Shape<Renderer R>", "renderer_bridge_", "R"));
        REQUIRE(IsField<Protected>(src, "Shape<Renderer R>", "x_", "double"));
        REQUIRE(IsField<Protected>(src, "Shape<Renderer R>", "y_", "double"));
        REQUIRE(
            IsField<Private>(src, "Circle<Renderer R>", "radius_", "double"));
        REQUIRE(
            IsField<Private>(src, "Rectangle<Renderer R>", "width_", "double"));
        REQUIRE(IsField<Private>(
            src, "Rectangle<Renderer R>", "height_", "double"));
        REQUIRE(IsField<Private>(
            src, "DrawingApplication<Renderer R>", "renderer_", "R"));

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
        REQUIRE(IsMethod<Public>(src, "Shape<Renderer R>", "draw", "void"));
        REQUIRE(IsMethod<Public>(
            src, "Shape<Renderer R>", "move", "void", "double dx, double dy"));
        REQUIRE(IsMethod<Public>(src, "Circle<Renderer R>", "draw", "void"));
        REQUIRE(IsMethod<Public>(
            src, "Circle<Renderer R>", "move", "void", "double dx, double dy"));
        REQUIRE(IsMethod<Public>(
            src, "Circle<Renderer R>", "set_radius", "void", "double radius"));
        REQUIRE(IsMethod<Public, Const>(
            src, "Circle<Renderer R>", "get_radius", "double"));
        REQUIRE(IsMethod<Public>(src, "Rectangle<Renderer R>", "draw", "void"));
        REQUIRE(IsMethod<Public>(src, "Rectangle<Renderer R>", "move", "void",
            "double dx, double dy"));
        REQUIRE(IsMethod<Public>(src, "Rectangle<Renderer R>", "set_dimensions",
            "void", "double width, double height"));
        REQUIRE(IsMethod<Public, Const>(
            src, "Rectangle<Renderer R>", "get_width", "double"));
        REQUIRE(IsMethod<Public, Const>(
            src, "Rectangle<Renderer R>", "get_height", "double"));
        REQUIRE(IsMethod<Public>(src, "DrawingApplication<Renderer R>",
            "create_circle", "void", "double radius"));
        REQUIRE(IsMethod<Public>(src, "DrawingApplication<Renderer R>",
            "create_rectangle", "void", "double width, double height"));
        REQUIRE(IsMethod<Public>(
            src, "DrawingApplication<Renderer R>", "draw_all"));
        REQUIRE(IsMethod<Public>(src, "DrawingApplication<Renderer R>",
            "move_all", "void", "double dx, double dy"));

        REQUIRE(IsBaseClass(src, "Shape<Renderer R>", "Circle<Renderer R>"));
        REQUIRE(IsBaseClass(src, "Shape<Renderer R>", "Rectangle<Renderer R>"));
    });
}