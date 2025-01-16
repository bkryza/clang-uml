/**
 * tests/t00004/test_case.h
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

TEST_CASE("t00004")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00004", "t00004_class");

    REQUIRE(diagram->include().namespaces.size() == 1);
    REQUIRE(diagram->exclude().namespaces.size() == 0);

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "A::AA"));
        REQUIRE(IsClass(src, "A::AA::AAA"));
        REQUIRE(IsEnum(src, "A::AA::Lights"));
        REQUIRE(IsInnerClass(src, "A", "A::AA"));
        REQUIRE(IsInnerClass(src, "A::AA", "A::AA::AAA"));
        REQUIRE(IsInnerClass(src, "A::AA", "A::AA::Lights"));

        REQUIRE(IsMethod<Public, Const>(src, "A", "foo"));
        REQUIRE(IsMethod<Public, Const>(src, "A", "foo2"));

        REQUIRE(IsEnum(src, "B::AA"));
        REQUIRE(IsEnum(src, "B::BB"));
        REQUIRE(IsEnum(src, "B::CC"));

#if LLVM_VERSION_MAJOR > 15
        REQUIRE(IsField<Public>(src, "B", "cc", "CC"));
        REQUIRE(IsField<Public>(src, "B", "bb", "BB"));
#else
        REQUIRE(IsField<Public>(src, "B", "cc", "B::CC"));
        REQUIRE(IsField<Public>(src, "B", "bb", "B::BB"));
#endif
        if constexpr (!std::is_same_v<graphml_t, std::decay_t<decltype(src)>>) {
            REQUIRE(!IsField<Public>(src, "B", "BB_1", "enum"));
            REQUIRE(!IsField<Public>(src, "B", "BB_2", "enum"));
            REQUIRE(!IsField<Public>(src, "B", "BB_3", "enum"));
        }

        REQUIRE(IsClassTemplate(src, "C<T>"));
        REQUIRE(IsInnerClass(src, "C<T>", "C::AA"));
        REQUIRE(IsInnerClass(src, "C::AA", "C::AA::AAA"));
        REQUIRE(IsInnerClass(src, "C<T>", "C::CC"));
        REQUIRE(IsInnerClass(src, "C::AA", "C::AA::CCC"));

        REQUIRE(IsInnerClass(src, "C<T>", "C::B<V>"));
        REQUIRE(
            IsAggregation<Public>(src, "C<T>", "C::B<int>", "b_int", "", "", "",
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t00004/t00004.cc#L48",
                    clanguml::util::get_git_commit()),
                "b_int"));
        REQUIRE(!IsInnerClass(src, "C<T>", "C::B"));
        REQUIRE(IsInstantiation(src, "C::B<V>", "C::B<int>"));

        REQUIRE(IsClass(src, {"detail", "D"}));
        REQUIRE(IsClass(src, {"detail", "D::DD"}));
        REQUIRE(IsEnum(src, {"detail", "D::AA"}));

        REQUIRE(IsAssociation<Public>(src, "B", "Color", "color", "", "", "",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t00004/t00004.cc#L15",
                clanguml::util::get_git_commit()),
            "color"));
        REQUIRE(IsAggregation<Public>(src, "B", "B::AA", "aa", "", "", "",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t00004/t00004.cc#L12",
                clanguml::util::get_git_commit()),
            "aa"));
        REQUIRE(IsAggregation<Private>(src, "A::AA::AAA", "A::AA::Lights",
            "lights", "", "", "",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t00004/t00004.cc#L27",
                clanguml::util::get_git_commit()),
            "lights"));

        REQUIRE(HasLink(src, "A::AA::AAA",
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t00004/t00004.cc#L26",
                clanguml::util::get_git_commit()),
            "A::AA::AAA"));
    });
}