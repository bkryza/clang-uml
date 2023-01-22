/**
 * tests/test_util.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
#define CATCH_CONFIG_MAIN

#include "util/util.h"
#include <cx/util.h>

#include <filesystem>

#include "catch.h"

TEST_CASE("Test split", "[unit-test]")
{
    using C = std::vector<std::string>;
    using namespace clanguml::util;

    const C empty{};

    CHECK(split("", " ") == C{""});
    CHECK(split("ABCD", " ") == C{"ABCD"});
    CHECK(split("::A", "::") == C{"A"});
    CHECK(split("::", "::") == C{});
    CHECK(split("A::", "::") == C{"A"});
    CHECK(split(":1", ":") == C{"1"});
    CHECK(split(":1", ":", false) == C{"", "1"});

    CHECK(split("std::vector::detail", "::") == C{"std", "vector", "detail"});

    CHECK(split("std::vector::detail::", "::") == C{"std", "vector", "detail"});
}

TEST_CASE("Test abbreviate", "[unit-test]")
{
    using namespace clanguml::util;

    CHECK(abbreviate("", 10) == "");
    CHECK(abbreviate("abcd", 10) == "abcd");
    CHECK(abbreviate("abcd", 2) == "ab");
    CHECK(abbreviate("abcdefg", 5) == "ab...");
}

TEST_CASE("Test starts_with", "[unit-test]")
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

TEST_CASE("Test replace_all", "[unit-test]")
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

TEST_CASE("Test parse_unexposed_template_params", "[unit-test]")
{
    using namespace clanguml::cx::util;

    const std::string int_template_str{"ns1::ns2::class1<int>"};

    auto int_template = parse_unexposed_template_params(
        int_template_str, [](const auto &n) { return n; });

    CHECK(int_template.size() == 1);
    CHECK(int_template[0].template_params().size() == 1);
    CHECK(int_template[0].type() == "ns1::ns2::class1");
    CHECK(int_template[0].template_params()[0].type() == "int");

    const std::string int_int_template_str{"ns1::ns2::class1<int, int>"};

    auto int_int_template = parse_unexposed_template_params(
        int_int_template_str, [](const auto &n) { return n; });

    CHECK(int_int_template.size() == 1);
    CHECK(int_int_template[0].template_params().size() == 2);
    CHECK(int_int_template[0].type() == "ns1::ns2::class1");
    CHECK(int_int_template[0].template_params()[0].type() == "int");
    CHECK(int_int_template[0].template_params()[1].type() == "int");

    const std::string nested_template_str{
        "class1<int, ns1::class2<int, std::vector<std::string>>>"};

    auto nested_template = parse_unexposed_template_params(
        nested_template_str, [](const auto &n) { return n; });

    CHECK(nested_template.size() == 1);
    CHECK(nested_template[0].template_params().size() == 2);
    CHECK(nested_template[0].type() == "class1");
    CHECK(nested_template[0].template_params()[0].type() == "int");
    const auto &class2 = nested_template[0].template_params()[1];
    CHECK(class2.type() == "ns1::class2");
    CHECK(class2.template_params()[0].type() == "int");
    CHECK(class2.template_params()[1].type() == "std::vector");
    CHECK(class2.template_params()[1].template_params()[0].type() ==
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
    CHECK(single_template[0].type() == "Else");

    const std::string declaration_string = R"(

            std::true_type, Result, Tail> {
                using type = Result;
            };)";

    auto declaration_template = parse_unexposed_template_params(
        declaration_string, [](const auto &n) { return n; });

    CHECK(declaration_template.size() == 3);
    CHECK(declaration_template[0].type() == "std::true_type");
    CHECK(declaration_template[1].type() == "Result");
    CHECK(declaration_template[2].type() == "Tail");
}

TEST_CASE("Test remove_prefix", "[unit-test]")
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

TEST_CASE("Test path_to_url", "[unit-test]")
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

TEST_CASE("Test ensure_path_is_absolute", "[unit-test]")
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

TEST_CASE("Test hash_seed", "[unit-test]")
{
    using namespace clanguml::util;

    CHECK(hash_seed(1) != 1);
    CHECK(hash_seed(1) == hash_seed(1));
    CHECK(hash_seed(1) != hash_seed(2));
}
