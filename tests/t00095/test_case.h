/**
 * tests/t00095/test_case.h
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

TEST_CASE("t00095")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00095", "t00095_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsAbstractClass(src, "Document"));
        REQUIRE(IsMethod<Public, Abstract, Const>(
            src, "Document", "clone", "std::unique_ptr<Document>"));
        REQUIRE(IsMethod<Public, Abstract>(
            src, "Document", "set_title", "void", "const std::string & title"));
        REQUIRE(IsMethod<Public, Abstract>(src, "Document", "set_content",
            "void", "const std::string & content"));
        REQUIRE(IsMethod<Public, Abstract, Const>(
            src, "Document", "get_title", "std::string"));
        REQUIRE(IsMethod<Public, Abstract, Const>(
            src, "Document", "get_content", "std::string"));
        REQUIRE(IsMethod<Public, Abstract, Const>(
            src, "Document", "get_type", "std::string"));
        REQUIRE(IsMethod<Public, Abstract, Const>(src, "Document", "display"));

        REQUIRE(IsField<Protected>(src, "Document", "title_", "std::string"));
        REQUIRE(IsField<Protected>(src, "Document", "content_", "std::string"));

        REQUIRE(IsClass(src, "TextDocument"));
        REQUIRE(IsClass(src, "SpreadsheetDocument"));
        REQUIRE(IsClass(src, "PresentationDocument"));

        REQUIRE(IsBaseClass(src, "Document", "TextDocument"));
        REQUIRE(IsBaseClass(src, "Document", "SpreadsheetDocument"));
        REQUIRE(IsBaseClass(src, "Document", "PresentationDocument"));

        REQUIRE(IsMethod<Public, Const>(
            src, "TextDocument", "clone", "std::unique_ptr<Document>"));
        REQUIRE(IsMethod<Public>(src, "TextDocument", "set_title", "void",
            "const std::string & title"));
        REQUIRE(IsMethod<Public>(src, "TextDocument", "set_content", "void",
            "const std::string & content"));
        REQUIRE(IsMethod<Public, Const>(
            src, "TextDocument", "get_title", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "TextDocument", "get_content", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "TextDocument", "get_type", "std::string"));
        REQUIRE(IsMethod<Public, Const>(src, "TextDocument", "display"));
        REQUIRE(IsMethod<Public>(src, "TextDocument", "set_font_family", "void",
            "const std::string & font"));
        REQUIRE(IsMethod<Public>(
            src, "TextDocument", "set_font_size", "void", "int size"));
        REQUIRE(IsMethod<Public, Const>(
            src, "TextDocument", "get_font_family", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "TextDocument", "get_font_size", "int"));

        REQUIRE(IsMethod<Public, Const>(
            src, "SpreadsheetDocument", "clone", "std::unique_ptr<Document>"));
        REQUIRE(IsMethod<Public>(src, "SpreadsheetDocument", "set_title",
            "void", "const std::string & title"));
        REQUIRE(IsMethod<Public>(src, "SpreadsheetDocument", "set_content",
            "void", "const std::string & content"));
        REQUIRE(IsMethod<Public, Const>(
            src, "SpreadsheetDocument", "get_title", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "SpreadsheetDocument", "get_content", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "SpreadsheetDocument", "get_type", "std::string"));
        REQUIRE(IsMethod<Public, Const>(src, "SpreadsheetDocument", "display"));
        REQUIRE(IsMethod<Public>(src, "SpreadsheetDocument", "set_dimensions",
            "void", "int rows, int columns"));
        REQUIRE(IsMethod<Public>(src, "SpreadsheetDocument", "add_formula",
            "void", "const std::string & cell, const std::string & formula"));
        REQUIRE(IsMethod<Public, Const>(
            src, "SpreadsheetDocument", "get_rows", "int"));
        REQUIRE(IsMethod<Public, Const>(
            src, "SpreadsheetDocument", "get_columns", "int"));

        REQUIRE(IsMethod<Public, Const>(
            src, "PresentationDocument", "clone", "std::unique_ptr<Document>"));
        REQUIRE(IsMethod<Public>(src, "PresentationDocument", "set_title",
            "void", "const std::string & title"));
        REQUIRE(IsMethod<Public>(src, "PresentationDocument", "set_content",
            "void", "const std::string & content"));
        REQUIRE(IsMethod<Public, Const>(
            src, "PresentationDocument", "get_title", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "PresentationDocument", "get_content", "std::string"));
        REQUIRE(IsMethod<Public, Const>(
            src, "PresentationDocument", "get_type", "std::string"));
        REQUIRE(
            IsMethod<Public, Const>(src, "PresentationDocument", "display"));
        REQUIRE(IsMethod<Public>(src, "PresentationDocument", "set_slide_count",
            "void", "int count"));
        REQUIRE(IsMethod<Public>(src, "PresentationDocument", "set_theme",
            "void", "const std::string & theme"));
        REQUIRE(IsMethod<Public>(src, "PresentationDocument", "add_slide_title",
            "void", "const std::string & title"));
        REQUIRE(IsMethod<Public, Const>(
            src, "PresentationDocument", "get_slide_count", "int"));
        REQUIRE(IsMethod<Public, Const>(
            src, "PresentationDocument", "get_theme", "std::string"));

        REQUIRE(IsClass(src, "DocumentRegistry"));
        REQUIRE(IsMethod<Public>(src, "DocumentRegistry", "register_prototype",
            "void",
            "const std::string & type, std::unique_ptr<Document> prototype"));
        REQUIRE(
            IsMethod<Public, Const>(src, "DocumentRegistry", "create_document",
                "std::unique_ptr<Document>", "const std::string & type"));
        REQUIRE(IsMethod<Public, Const>(src, "DocumentRegistry",
            "get_registered_types", "std::vector<std::string>"));
        REQUIRE(IsMethod<Public, Const>(src, "DocumentRegistry",
            "is_type_registered", "bool", "const std::string & type"));

        // Check aggregation relationship between Registry and Document
        REQUIRE(IsAggregation<Private>(
            src, "DocumentRegistry", "Document", "prototypes_"));

        REQUIRE(IsClass(src, "DocumentFactory"));
        REQUIRE(IsMethod<Public>(src, "DocumentFactory", "create_text_document",
            "std::unique_ptr<Document>"));
        REQUIRE(IsMethod<Public>(src, "DocumentFactory",
            "create_spreadsheet_document", "std::unique_ptr<Document>"));
        REQUIRE(IsMethod<Public>(src, "DocumentFactory",
            "create_presentation_document", "std::unique_ptr<Document>"));
        REQUIRE(
            IsMethod<Public>(src, "DocumentFactory", "create_document_by_type",
                "std::unique_ptr<Document>", "const std::string & type"));
        REQUIRE(IsMethod<Public>(src, "DocumentFactory",
            "register_custom_prototype", "void",
            "const std::string & type, std::unique_ptr<Document> prototype"));
        REQUIRE(IsMethod<Public, Const>(src, "DocumentFactory",
            "get_available_types", "std::vector<std::string>"));

        REQUIRE(IsAggregation<Private>(
            src, "DocumentFactory", "DocumentRegistry", "registry_"));

        REQUIRE(IsClass(src, "DocumentApplication"));
        REQUIRE(
            IsMethod<Public>(src, "DocumentApplication", "create_new_document",
                "std::unique_ptr<Document>", "const std::string & type"));
        REQUIRE(
            IsMethod<Public>(src, "DocumentApplication", "duplicate_document",
                "std::unique_ptr<Document>", "const Document & original"));
        REQUIRE(IsMethod<Public>(src, "DocumentApplication",
            "create_document_template", "void",
            "const std::string & name, std::unique_ptr<Document> "
            "template_doc"));

        REQUIRE(IsAggregation<Private>(
            src, "DocumentApplication", "DocumentFactory", "factory_"));

        REQUIRE(IsDependency(src, "DocumentFactory", "Document"));
        REQUIRE(IsDependency(src, "DocumentApplication", "Document"));
    });
}