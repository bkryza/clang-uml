/**
 * tests/t00018/test_case.cc
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

TEST_CASE("t00018", "[test-case][class]")
{
    auto [config, db] = load_config("t00018");

    auto diagram = config.diagrams["t00018_class"];

    REQUIRE(diagram->name == "t00018_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00018_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));
        REQUIRE_THAT(src, IsClass(_A("widget")));
        REQUIRE_THAT(src, IsClass(_A("impl::widget")));

        REQUIRE_THAT(
            src, IsAggregation(_A("widget"), _A("impl::widget"), "-pImpl"));
        REQUIRE_THAT(src, IsDependency(_A("impl::widget"), _A("widget")));
        REQUIRE_THAT(src, !IsDependency(_A("widget"), _A("widget")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "widget"));
        REQUIRE(IsClass(j, "impl::widget"));
        REQUIRE(IsDependency(j, "impl::widget", "widget"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);
        mermaid::AliasMatcher _A(src);

        REQUIRE_THAT(src, IsClass(_A("widget")));
        REQUIRE_THAT(src, IsClass(_A("impl::widget")));

        REQUIRE_THAT(
            src, IsAggregation(_A("widget"), _A("impl::widget"), "-pImpl"));
        REQUIRE_THAT(src, IsDependency(_A("impl::widget"), _A("widget")));
        REQUIRE_THAT(src, !IsDependency(_A("widget"), _A("widget")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}