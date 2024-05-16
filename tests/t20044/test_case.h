/**
 * tests/t20044/test_case.h
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

TEST_CASE("t20044")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20044", "t20044_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "R", "R((lambda at t20044.cc:74:9) &&)"}, //
                {"R", "tmain()::(lambda t20044.cc:74:9)",
                    "operator()() const"},                              //
                {"tmain()::(lambda t20044.cc:74:9)", "A", "a() const"}, //

                {"tmain()", "tmain()::(lambda t20044.cc:84:18)",
                    "operator()() const"},                          //
                {"tmain()::(lambda t20044.cc:84:18)", "A", "a5()"}, //

                {"tmain()", "A", "a1() const"},                        //
                {"A", "detail::expected<int,error>", "expected(int)"}, //

                {"tmain()", "detail::expected<int,error>",
                    "and_then((lambda at t20044.cc:90:19) &&)"}, //
                {"detail::expected<int,error>",
                    "tmain()::(lambda t20044.cc:90:19)",
                    "operator()(auto &&) const"},                            //
                {"tmain()::(lambda t20044.cc:90:19)", "A", "a2(int) const"}, //
                {"A", "detail::expected<int,error>", "expected(int)"},       //

                {"tmain()", "detail::expected<int,error>",
                    "and_then(result_t (&)(int))"}, //                                                            //
                {"tmain()", "detail::expected<int,error>",
                    "and_then(std::function<result_t (int)> &)"}, //                                                            //
                {"tmain()", "detail::expected<int,error>",
                    "value() const"}, //                                                            //

            }));
    });
/*
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src,
            HasCall(
                _A("tmain()"), _A("R"), "R((lambda at t20044.cc:74:9) &&)"));
        REQUIRE_THAT(src,
            HasCall(_A("R"), _A("tmain()::(lambda t20044.cc:74:9)"),
                "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20044.cc:74:9)"), _A("A"), "a()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("tmain()::(lambda t20044.cc:84:18)"),
                "operator()()"));
        REQUIRE_THAT(src,
            HasCall(_A("tmain()::(lambda t20044.cc:84:18)"), _A("A"), "a5()"));

        REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "a1()"));

        REQUIRE_THAT(src,
            HasCall(_A("tmain()"), _A("detail::expected<int,error>"),
                "and_then((lambda at t20044.cc:90:19) &&)"));

        REQUIRE_THAT(src,
            HasCall(_A("detail::expected<int,error>"),
                _A("tmain()::(lambda t20044.cc:90:19)"),
                "operator()(auto &&) const"));

        REQUIRE_THAT(src,
            HasCall(
                _A("tmain()::(lambda t20044.cc:90:19)"), _A("A"), "a2(int)"));

        REQUIRE_THAT(src,
            HasCall(
                _A("A"), _A("detail::expected<int,error>"), "expected(int)"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsClass;

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}