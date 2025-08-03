/**
 * tests/t00101/test_case.h
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

TEST_CASE("t00101")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00101", "t00101_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsConcept(src, "FormattingStrategyConcept<T>"));
        REQUIRE(IsConceptRequirement(src, "FormattingStrategyConcept<T>",
            "{t.format(text)} -> std::same_as<std::string>"));
        REQUIRE(IsConceptRequirement(src, "FormattingStrategyConcept<T>",
            "{t.get_name()} -> std::same_as<std::string>"));

        REQUIRE(IsClassTemplate(src, "FormattingStrategyBase<Derived>"));
        REQUIRE(IsMethod<Public>(src, "FormattingStrategyBase<Derived>",
            "format", "std::string", "const std::string & text"));
        REQUIRE(IsMethod<Public>(
            src, "FormattingStrategyBase<Derived>", "get_name", "std::string"));

        REQUIRE(IsClass(src, "PlainTextStrategy"));
        REQUIRE(IsClass(src, "MarkdownStrategy"));

        REQUIRE(IsBaseClass(src, "FormattingStrategyBase<PlainTextStrategy>",
            "PlainTextStrategy"));
        REQUIRE(IsBaseClass(src, "FormattingStrategyBase<MarkdownStrategy>",
            "MarkdownStrategy"));

        REQUIRE(IsMethod<Public>(src, "PlainTextStrategy", "format_impl",
            "std::string", "const std::string & text"));
        REQUIRE(IsMethod<Public>(
            src, "PlainTextStrategy", "get_name_impl", "std::string"));

        REQUIRE(IsMethod<Public>(src, "MarkdownStrategy", "format_impl",
            "std::string", "const std::string & text"));
        REQUIRE(IsMethod<Public>(
            src, "MarkdownStrategy", "get_name_impl", "std::string"));

        REQUIRE(IsClassTemplate(
            src, "TextProcessor<FormattingStrategyConcept StrategyType>"));
        REQUIRE(IsMethod<Public>(src,
            "TextProcessor<FormattingStrategyConcept StrategyType>",
            "process_text", "std::string", "const std::string & text"));
        REQUIRE(IsMethod<Public>(src,
            "TextProcessor<FormattingStrategyConcept StrategyType>",
            "get_strategy_name", "std::string"));
        REQUIRE(IsMethod<Public>(src,
            "TextProcessor<FormattingStrategyConcept StrategyType>",
            "set_strategy", "void", "StrategyType & new_strategy"));

        REQUIRE(IsClass(src, "DocumentProcessor"));
        REQUIRE(
            IsMethod<Public>(src, "DocumentProcessor", "process_as_plain_text",
                "std::string", "const std::string & content"));
        REQUIRE(
            IsMethod<Public>(src, "DocumentProcessor", "process_as_markdown",
                "std::string", "const std::string & content"));
        REQUIRE(IsMethod<Public>(src, "DocumentProcessor",
            "process_with_custom_strategy<FormattingStrategyConcept "
            "StrategyType>",
            "std::string",
            "const std::string & content, StrategyType & strategy"));

        REQUIRE(IsClass(src, "StrategyFactory"));
        REQUIRE(IsEnum(src, "StrategyFactory::StrategyType"));
        REQUIRE(IsMethod<Public, Static>(src, "StrategyFactory",
            "create_plain_text_strategy",
            "std::unique_ptr<PlainTextStrategy>"));
        REQUIRE(IsMethod<Public, Static>(src, "StrategyFactory",
            "create_markdown_strategy", "std::unique_ptr<MarkdownStrategy>"));

        REQUIRE(IsDependency(src, "StrategyFactory", "PlainTextStrategy"));
        REQUIRE(IsDependency(src, "StrategyFactory", "MarkdownStrategy"));
    });
}