/**
 * tests/t00096/test_case.h
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

TEST_CASE("t00096")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00096", "t00096_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsConcept(src, "Cloneable<T>"));
        REQUIRE(IsConcept(src, "Drawable<T>"));
        REQUIRE(IsConcept(src, "CloneableDrawable<T>"));

        REQUIRE(IsClassTemplate(src, "CloneableBase<Derived>"));
        REQUIRE(IsMethod<Public, Const>(src, "CloneableBase<Derived>", "clone",
            "std::unique_ptr<Derived>"));

        REQUIRE(IsAbstractClass(src, "Shape"));
        REQUIRE(IsMethod<Public, Abstract, Const>(src, "Shape", "draw"));
        REQUIRE(IsMethod<Public, Abstract, Const>(
            src, "Shape", "get_area", "double"));
        REQUIRE(IsMethod<Public, Abstract, Const>(
            src, "Shape", "get_type_name", "std::string"));
        REQUIRE(IsMethod<Public>(
            src, "Shape", "set_position", "void", "double x, double y"));
        REQUIRE(IsMethod<Public, Const>(src, "Shape", "get_x", "double"));
        REQUIRE(IsMethod<Public, Const>(src, "Shape", "get_y", "double"));

        REQUIRE(IsField<Protected>(src, "Shape", "x_", "double"));
        REQUIRE(IsField<Protected>(src, "Shape", "y_", "double"));

        REQUIRE(IsClass(src, "Circle"));
        REQUIRE(IsClass(src, "Rectangle"));
        REQUIRE(IsClass(src, "Triangle"));

        REQUIRE(IsBaseClass(src, "Shape", "Circle"));
        REQUIRE(IsBaseClass(src, "Shape", "Rectangle"));
        REQUIRE(IsBaseClass(src, "Shape", "Triangle"));

        REQUIRE(IsBaseClass(src, "CloneableBase<Circle>", "Circle"));
        REQUIRE(IsBaseClass(src, "CloneableBase<Rectangle>", "Rectangle"));
        REQUIRE(IsBaseClass(src, "CloneableBase<Triangle>", "Triangle"));

        REQUIRE(IsMethod<Public, Const>(src, "Circle", "draw"));
        REQUIRE(IsMethod<Public, Const>(src, "Circle", "get_area", "double"));
        REQUIRE(IsMethod<Public, Const>(
            src, "Circle", "get_type_name", "std::string"));
        REQUIRE(IsMethod<Public>(
            src, "Circle", "set_radius", "void", "double radius"));
        REQUIRE(IsMethod<Public, Const>(src, "Circle", "get_radius", "double"));

        REQUIRE(IsMethod<Public, Const>(src, "Rectangle", "draw"));
        REQUIRE(
            IsMethod<Public, Const>(src, "Rectangle", "get_area", "double"));
        REQUIRE(IsMethod<Public, Const>(
            src, "Rectangle", "get_type_name", "std::string"));
        REQUIRE(IsMethod<Public>(src, "Rectangle", "set_dimensions", "void",
            "double width, double height"));
        REQUIRE(
            IsMethod<Public, Const>(src, "Rectangle", "get_width", "double"));
        REQUIRE(
            IsMethod<Public, Const>(src, "Rectangle", "get_height", "double"));

        REQUIRE(IsMethod<Public, Const>(src, "Triangle", "draw"));
        REQUIRE(IsMethod<Public, Const>(src, "Triangle", "get_area", "double"));
        REQUIRE(IsMethod<Public, Const>(
            src, "Triangle", "get_type_name", "std::string"));
        REQUIRE(IsMethod<Public>(src, "Triangle", "set_dimensions", "void",
            "double base, double height"));
        REQUIRE(IsMethod<Public, Const>(src, "Triangle", "get_base", "double"));
        REQUIRE(
            IsMethod<Public, Const>(src, "Triangle", "get_height", "double"));

        REQUIRE(IsClassTemplate(src, "PrototypeRegistry<CloneableDrawable T>"));
        REQUIRE(IsMethod<Public>(src, "PrototypeRegistry<CloneableDrawable T>",
            "register_prototype", "void",
            "const std::string & name, const T & prototype"));
        REQUIRE(IsMethod<Public, Const>(src,
            "PrototypeRegistry<CloneableDrawable T>", "create_clone",
            "std::unique_ptr<T>", "const std::string & name"));
        REQUIRE(IsMethod<Public, Const>(src,
            "PrototypeRegistry<CloneableDrawable T>", "has_prototype", "bool",
            "const std::string & name"));
        REQUIRE(IsMethod<Public, Const>(src,
            "PrototypeRegistry<CloneableDrawable T>", "get_prototype_names",
            "std::vector<std::string>"));
        REQUIRE(IsMethod<Public, Const>(
            src, "PrototypeRegistry<CloneableDrawable T>", "size", "size_t"));

        REQUIRE(
            IsClassTemplate(src, "ShapeFactory<CloneableDrawable Types...>"));
        REQUIRE(IsMethod<Public, Const>(src,
            "ShapeFactory<CloneableDrawable Types...>", "create_circle",
            "std::unique_ptr<Circle>", "const std::string & name"));
        REQUIRE(IsMethod<Public, Const>(src,
            "ShapeFactory<CloneableDrawable Types...>", "create_rectangle",
            "std::unique_ptr<Rectangle>", "const std::string & name"));
        REQUIRE(IsMethod<Public, Const>(src,
            "ShapeFactory<CloneableDrawable Types...>", "create_triangle",
            "std::unique_ptr<Triangle>", "const std::string & name"));

        REQUIRE(IsAggregation<Private>(src,
            "ShapeFactory<CloneableDrawable Types...>",
            "PrototypeRegistry<Circle>", "circle_registry_"));
        REQUIRE(IsAggregation<Private>(src,
            "ShapeFactory<CloneableDrawable Types...>",
            "PrototypeRegistry<Rectangle>", "rectangle_registry_"));
        REQUIRE(IsAggregation<Private>(src,
            "ShapeFactory<CloneableDrawable Types...>",
            "PrototypeRegistry<Triangle>", "triangle_registry_"));

        REQUIRE(IsClassTemplate(src, "ShapeManager<CloneableDrawable T>"));
        REQUIRE(IsMethod<Public>(src, "ShapeManager<CloneableDrawable T>",
            "add_prototype", "void",
            "const std::string & name, const T & prototype"));
        REQUIRE(IsMethod<Public, Const>(src,
            "ShapeManager<CloneableDrawable T>", "create_from_prototype",
            "std::unique_ptr<T>", "const std::string & name"));
        REQUIRE(
            IsMethod<Public, Const>(src, "ShapeManager<CloneableDrawable T>",
                "create_multiple", "std::vector<std::unique_ptr<T>>",
                "const std::string & name, size_t count"));
        REQUIRE(IsMethod<Public, Const>(src,
            "ShapeManager<CloneableDrawable T>", "calculate_total_area",
            "double", "const std::vector<std::unique_ptr<T>> & shapes"));

        REQUIRE(IsAggregation<Private>(src, "ShapeManager<CloneableDrawable T>",
            "PrototypeRegistry<T>", "registry_"));

        REQUIRE(IsClass(src, "DrawingApplication"));
        REQUIRE(IsMethod<Public, Const>(src, "DrawingApplication",
            "create_circle_from_template", "std::unique_ptr<Circle>",
            "const std::string & template_name"));
        REQUIRE(IsMethod<Public, Const>(src, "DrawingApplication",
            "create_rectangle_from_template", "std::unique_ptr<Rectangle>",
            "const std::string & template_name"));
        REQUIRE(IsMethod<Public, Const>(src, "DrawingApplication",
            "create_triangle_from_template", "std::unique_ptr<Triangle>",
            "const std::string & template_name"));

        REQUIRE(IsAggregation<Private>(src, "DrawingApplication",
            "ShapeFactory<Circle,Rectangle,Triangle>", "factory_"));

        REQUIRE(IsInstantiation(
            src, "CloneableBase<Derived>", "CloneableBase<Circle>"));
        REQUIRE(IsInstantiation(
            src, "CloneableBase<Derived>", "CloneableBase<Rectangle>"));
        REQUIRE(IsInstantiation(
            src, "CloneableBase<Derived>", "CloneableBase<Triangle>"));

        REQUIRE(IsInstantiation(src, "PrototypeRegistry<CloneableDrawable T>",
            "PrototypeRegistry<Circle>"));
        REQUIRE(IsInstantiation(src, "PrototypeRegistry<CloneableDrawable T>",
            "PrototypeRegistry<Rectangle>"));
        REQUIRE(IsInstantiation(src, "PrototypeRegistry<CloneableDrawable T>",
            "PrototypeRegistry<Triangle>"));

        REQUIRE(IsInstantiation(src, "ShapeFactory<CloneableDrawable Types...>",
            "ShapeFactory<Circle,Rectangle,Triangle>"));

        REQUIRE(IsConstraint(src, "PrototypeRegistry<CloneableDrawable T>",
            "CloneableDrawable<T>", "T"));
        REQUIRE(IsConstraint(src, "ShapeFactory<CloneableDrawable Types...>",
            "CloneableDrawable<T>", "Types..."));
        REQUIRE(IsConstraint(src, "ShapeManager<CloneableDrawable T>",
            "CloneableDrawable<T>", "T"));
    });
}