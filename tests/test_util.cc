/**
 * @file tests/test_util.cc
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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "util/util.h"
#include <common/clang_utils.h>

#include <filesystem>

#include "doctest/doctest.h"

TEST_CASE("Test split")
{
    using C = std::vector<std::string>;
    using namespace clanguml::util;

    const C empty{};

    CHECK(split("", " ") == C{});
    CHECK(split("", ".") == C{});
    CHECK(split("ABCD", " ") == C{"ABCD"});
    CHECK(split("::A", "::") == C{"A"});
    CHECK(split("::", "::") == C{});
    CHECK(split("A::", "::") == C{"A"});
    CHECK(split(":1", ":") == C{"1"});
    CHECK(split(":1", ":", false) == C{"", "1"});

    CHECK(split("std::vector::detail", "::") == C{"std", "vector", "detail"});

    CHECK(split("std::vector::detail::", "::") == C{"std", "vector", "detail"});
}

TEST_CASE("Test abbreviate")
{
    using namespace clanguml::util;

    CHECK(abbreviate("", 10) == "");
    CHECK(abbreviate("abcd", 10) == "abcd");
    CHECK(abbreviate("abcd", 2) == "ab");
    CHECK(abbreviate("abcdefg", 5) == "ab...");
}

TEST_CASE("Test starts_with")
{
    using clanguml::util::starts_with;
    using std::filesystem::path;

    CHECK(starts_with(path{"/a/b/c/d"}, path{"/"}));
    CHECK_FALSE(starts_with(path{"/a/b/c/d"}, path{"/a/b/c/d/e"}));
    CHECK(starts_with(path{"/a/b/c/d/e"}, path{"/a/b/c/d"}));
    CHECK(starts_with(path{"/a/b/c/d/e"}, path{"/a/b/c/d/"}));
    CHECK_FALSE(starts_with(path{"/e/f/c/d/file.h"}, path{"/a/b"}));
    CHECK_FALSE(starts_with(path{"/e/f/c/d/file.h"}, path{"/a/b/"}));
    CHECK(starts_with(path{"/a/b/c/d/file.h"}, path{"/a/b/c"}));
    CHECK(starts_with(path{"/a/b/c/file.h"}, path{"/a/b/c/file.h"}));
    CHECK(starts_with(path{"c/file.h"}, path{"c"}));
    CHECK(starts_with(path{"c/file.h"}, path{"c/"}));
    CHECK_FALSE(starts_with(path{"c/file1.h"}, path{"c/file2.h"}));
}

TEST_CASE("Test replace_all")
{
    using namespace clanguml::util;

    const std::string orig = R"(
                        Lorem ipsum \clanguml{note} something something...

                        \clanguml{style}

                        )";
    std::string text{orig};

    CHECK(replace_all(text, "NOTHERE", "HERE") == false);
    CHECK(replace_all(text, "\\clanguml", "@clanguml") == true);
    CHECK(replace_all(text, "something", "nothing") == true);
    CHECK(replace_all(text, "something", "nothing") == false);
    CHECK(replace_all(text, "nothing", "something") == true);
    CHECK(replace_all(text, "@clanguml", "\\clanguml") == true);

    CHECK(text == orig);
}

TEST_CASE("Test extract_template_parameter_index")
{
    using namespace clanguml::common;

    {
        const auto [depth, index, qualifier] =
            extract_template_parameter_index("type-parameter-0-0");
        CHECK(depth == 0);
        CHECK(index == 0);
        CHECK(qualifier.empty());
    }

    {
        const auto [depth, index, qualifier] =
            extract_template_parameter_index("type-parameter-0-0 &&");
        CHECK(depth == 0);
        CHECK(index == 0);
        CHECK(qualifier == "&&");
    }

    {
        const auto [depth, index, qualifier] =
            extract_template_parameter_index("type-parameter-12-678 const&");
        CHECK(depth == 12);
        CHECK(index == 678);
        CHECK(qualifier == "const&");
    }
}

TEST_CASE("Test parse_unexposed_template_params")
{
    using namespace clanguml::common;

    const std::string int_template_str{"ns1::ns2::class1<int>"};

    auto int_template = parse_unexposed_template_params(
        int_template_str, [](const auto &n) { return n; });

    CHECK(int_template.size() == 1);
    CHECK(int_template[0].template_params().size() == 1);
    CHECK(int_template[0].type().value() == "ns1::ns2::class1");
    CHECK(int_template[0].template_params()[0].type().value() == "int");

    const std::string int_int_template_str{"ns1::ns2::class1<int, int>"};

    auto int_int_template = parse_unexposed_template_params(
        int_int_template_str, [](const auto &n) { return n; });

    CHECK(int_int_template.size() == 1);
    CHECK(int_int_template[0].template_params().size() == 2);
    CHECK(int_int_template[0].type().value() == "ns1::ns2::class1");
    CHECK(int_int_template[0].template_params()[0].type().value() == "int");
    CHECK(int_int_template[0].template_params()[1].type().value() == "int");

    const std::string nested_template_str{
        "class1<int, ns1::class2<int, std::vector<std::string>>>"};

    auto nested_template = parse_unexposed_template_params(
        nested_template_str, [](const auto &n) { return n; });

    CHECK(nested_template.size() == 1);
    CHECK(nested_template[0].template_params().size() == 2);
    CHECK(nested_template[0].type().value() == "class1");
    CHECK(nested_template[0].template_params()[0].type().value() == "int");
    const auto &class2 = nested_template[0].template_params()[1];
    CHECK(class2.type() == "ns1::class2");
    CHECK(class2.template_params()[0].type().value() == "int");
    CHECK(class2.template_params()[1].type().value() == "std::vector");
    CHECK(class2.template_params()[1].template_params()[0].type().value() ==
        "std::string");

    const std::string empty_string = R"(
            > {
                using type = Result;
            };)";

    auto empty_template = parse_unexposed_template_params(
        empty_string, [](const auto &n) { return n; });

    CHECK(empty_template.size() == 0);

    const std::string single_template_string = R"(Else> {
            using type = Else;)";

    auto single_template = parse_unexposed_template_params(
        single_template_string, [](const auto &n) { return n; });

    CHECK(single_template.size() == 1);
    CHECK(single_template[0].type().value() == "Else");

    const std::string declaration_string = R"(

            std::true_type, Result, Tail> {
                using type = Result;
            };)";

    auto declaration_template = parse_unexposed_template_params(
        declaration_string, [](const auto &n) { return n; });

    CHECK(declaration_template.size() == 3);
    CHECK(declaration_template[0].type().value() == "std::true_type");
    CHECK(declaration_template[1].type().value() == "Result");
    CHECK(declaration_template[2].type().value() == "Tail");
}

TEST_CASE("Test remove_prefix")
{
    using namespace clanguml::util;

    const std::vector<std::string> collection_base = {"aa", "bb", "cc", "dd"};
    std::vector<std::string> collection = collection_base;

    const std::vector<std::string> prefix1 = {"xx", "yy"};
    const std::vector<std::string> prefix2 = {"aa", "bb"};
    const std::vector<std::string> prefix3 = {"cc", "dd"};

    remove_prefix(collection, prefix1);

    CHECK(collection == collection_base);

    remove_prefix(collection, prefix2);

    CHECK(collection == prefix3);

    remove_prefix(collection, prefix3);

    CHECK(collection.empty());
}

TEST_CASE("Test path_to_url")
{
    namespace fs = std::filesystem;
    using namespace clanguml::util;

    fs::path p1{""};
    p1.make_preferred();

    CHECK(path_to_url(p1) == "");

    fs::path p2{"a/b/c/d"};
    p2.make_preferred();
    CHECK(path_to_url(p2) == "a/b/c/d");

    fs::path p3{"/a/b/c/d"};
    p3.make_preferred();
    CHECK(path_to_url(p3) == "/a/b/c/d");

    fs::path p4{"/"};
    p4.make_preferred();
    CHECK(path_to_url(p4) == "/");

#ifdef _MSC_VER
    fs::path p5{"C:\\A\\B\\include.h"};
    CHECK(path_to_url(p5) == "/c/A/B/include.h");

    fs::path p6{"C:A\\B\\include.h"};
    CHECK(path_to_url(p6) == "C:/A/B/include.h");

    fs::path p7{"A\\B\\include.h"};
    CHECK(path_to_url(p7) == "A/B/include.h");
#endif
}

TEST_CASE("Test ensure_path_is_absolute")
{
    using namespace clanguml::util;

    using std::filesystem::path;

    CHECK(ensure_path_is_absolute("a/b/c", "/tmp").string() ==
        path{"/tmp/a/b/c"}.make_preferred());
    CHECK(ensure_path_is_absolute("/a/b/c", "/tmp").string() ==
        path{"/a/b/c"}.make_preferred());
    CHECK(ensure_path_is_absolute("", "/tmp").string() ==
        path{"/tmp/"}.make_preferred());
    CHECK(ensure_path_is_absolute(".", "/tmp").string() ==
        path{"/tmp/"}.make_preferred());
    CHECK(ensure_path_is_absolute("..", "/tmp").string() ==
        path{"/"}.make_preferred());
    CHECK(ensure_path_is_absolute("/", "/tmp").string() ==
        path{"/"}.make_preferred());
}

TEST_CASE("Test hash_seed")
{
    using namespace clanguml::util;

    CHECK(hash_seed(1) != 1);
    CHECK(hash_seed(1) == hash_seed(1));
    CHECK(hash_seed(1) != hash_seed(2));
}

TEST_CASE("Test tokenize_unexposed_template_parameter")
{
    using namespace clanguml::common;

    {
        auto r = tokenize_unexposed_template_parameter("type-parameter-0-1");
        CHECK(r[0] == "type-parameter-0-1");
    }

    {
        int i = 0;

        auto r =
            tokenize_unexposed_template_parameter("const type-parameter-0-1 &");
        CHECK(r[i++] == "const");
        CHECK(r[i++] == "type-parameter-0-1");
        CHECK(r[i++] == "&");
    }

    {
        int i = 0;

        auto r = tokenize_unexposed_template_parameter(
            "const type-parameter-0-0 type-parameter-0-1::* &&");
        CHECK(r[i++] == "const");
        CHECK(r[i++] == "type-parameter-0-0");
        CHECK(r[i++] == "type-parameter-0-1");
        CHECK(r[i++] == "::");
        CHECK(r[i++] == "*");
        CHECK(r[i++] == "&&");
    }

    {
        int i = 0;

        auto r = tokenize_unexposed_template_parameter(
            "type-parameter-0-0 [type-parameter-0-1]");
        CHECK(r[i++] == "type-parameter-0-0");
        CHECK(r[i++] == "[");
        CHECK(r[i++] == "type-parameter-0-1");
        CHECK(r[i++] == "]");
    }

    {
        int i = 0;
        auto r = tokenize_unexposed_template_parameter(
            "type-parameter-0-0 (type-parameter-0-1::*)(type-parameter-0-2, "
            "type-parameter-0-3, type-parameter-0-4)");
        CHECK(r[i++] == "type-parameter-0-0");
        CHECK(r[i++] == "(");
        CHECK(r[i++] == "type-parameter-0-1");
        CHECK(r[i++] == "::");
        CHECK(r[i++] == "*");
        CHECK(r[i++] == ")");
        CHECK(r[i++] == "(");
        CHECK(r[i++] == "type-parameter-0-2");
        CHECK(r[i++] == ",");
        CHECK(r[i++] == "type-parameter-0-3");
        CHECK(r[i++] == ",");
        CHECK(r[i++] == "type-parameter-0-4");
        CHECK(r[i++] == ")");
    }

    {
        int i = 0;

        auto r = tokenize_unexposed_template_parameter("const ns1::ns2::A &");
        CHECK(r[i++] == "const");
        CHECK(r[i++] == "ns1::ns2::A");
        CHECK(r[i++] == "&");
    }

    {
        int i = 0;

        auto r = tokenize_unexposed_template_parameter("class ns1::ns2::A &");
        CHECK(r[i++] == "ns1::ns2::A");
        CHECK(r[i++] == "&");
    }

    {
        int i = 0;

        auto r = tokenize_unexposed_template_parameter("ns1::ns2::A C::* &&");
        CHECK(r[i++] == "ns1::ns2::A");
        CHECK(r[i++] == "C");
        CHECK(r[i++] == "::");
        CHECK(r[i++] == "*");
        CHECK(r[i++] == "&&");
    }

    {
        int i = 0;

        auto r = tokenize_unexposed_template_parameter("unsigned int");
        CHECK(r[i++] == "unsigned");
        CHECK(r[i++] == "int");
    }

    {
        int i = 0;

        auto r = tokenize_unexposed_template_parameter("Ret(Args...)");
        CHECK(r[i++] == "Ret");
        CHECK(r[i++] == "(");
        CHECK(r[i++] == "Args");
        CHECK(r[i++] == "...");
        CHECK(r[i++] == ")");
    }
}

TEST_CASE("Test format_message_comment")
{
    using namespace clanguml::util;

    CHECK(format_message_comment("\n\n   ABCD \n", 0) == "\n\n   ABCD \n");
    CHECK(format_message_comment("\n\n   ABCD \n") == "ABCD");

    CHECK(format_message_comment(
              "Repeat until send() succeeds or retry count is exceeded") ==
        "Repeat until send() succeeds\nor retry count is exceeded");

    CHECK(format_message_comment("12345678", 5) == "12345678");

    CHECK(format_message_comment("") == "");

    CHECK(format_message_comment(" ") == "");

    CHECK(format_message_comment("This is a url: http://example.com/test/12345",
              15) == "This is a url:\nhttp://example.com/test/12345");
}

TEST_CASE("Test is_relative_to")
{
    using std::filesystem::path;
    using namespace clanguml::util;

    path child{"/a/b/c/d/include.h"};
    path base1{"/a/b/c"};
    path base2{"/tmp"};

    CHECK(is_relative_to(child, base1));
    CHECK_FALSE(is_relative_to(child, base2));
}

TEST_CASE("Test parse_source_location")
{
    using namespace clanguml::common;

    const std::string int_template_str{"ns1::ns2::class1<int>"};

    std::string file;
    unsigned line{};
    unsigned column{};
    bool result = false;

    result = parse_source_location("/a/b/c/d/test.cpp:123", file, line, column);

    CHECK_FALSE(result);

    result = false, file = "", line = 0, column = 0;
    result =
        parse_source_location("/a/b/c/d/test.cpp:123:456", file, line, column);

    CHECK(result);
    CHECK(file == "/a/b/c/d/test.cpp");
    CHECK(line == 123);
    CHECK(column == 456);

    result = false, file = "", line = 0, column = 0;

    result = parse_source_location(
        "E:\\test\\src\\main.cpp:123", file, line, column);

    CHECK_FALSE(result);

    result = false, file = "", line = 0, column = 0;

    result = parse_source_location(
        "E:\\test\\src\\main.cpp:123:456", file, line, column);

    CHECK(result);
    CHECK(file == "E:\\test\\src\\main.cpp");
    CHECK(line == 123);
    CHECK(column == 456);

    result = false, file = "", line = 0, column = 0;
}