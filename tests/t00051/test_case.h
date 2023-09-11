/**
 * tests/t00051/test_case.h
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

TEST_CASE("t00051", "[test-case][class]")
{
    auto [config, db] = load_config("t00051");

    auto diagram = config.diagrams["t00051_class"];

    REQUIRE(diagram->name == "t00051_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00051_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsInnerClass(_A("A"), _A("A::custom_thread1")));
        REQUIRE_THAT(src, IsInnerClass(_A("A"), _A("A::custom_thread2")));

        REQUIRE_THAT(src,
            (IsMethod<Public>("custom_thread1<Function,Args...>", "void",
                "Function && f, Args &&... args")));
        REQUIRE_THAT(src,
            (IsMethod<Public>("thread", "void",
                "(lambda at ../../tests/t00051/t00051.cc:59:27) &&")));
        REQUIRE_THAT(src,
            (IsMethod<Private>("start_thread3",
                "B<(lambda at ../../tests/t00051/t00051.cc:43:18),(lambda at "
                "../../tests/t00051/t00051.cc:43:27)>")));
        REQUIRE_THAT(src,
            (IsMethod<Private>("get_function",
                "(lambda at ../../tests/t00051/t00051.cc:48:16)")));

        REQUIRE_THAT(src, IsClassTemplate("B", "F,FF=F"));
        REQUIRE_THAT(src, (IsMethod<Public>("f", "void")));
        REQUIRE_THAT(src, (IsMethod<Public>("ff", "void")));

        REQUIRE_THAT(src,
            IsClassTemplate("B",
                "(lambda at ../../tests/t00051/t00051.cc:43:18),(lambda at "
                "../../tests/t00051/t00051.cc:43:27)"));

        REQUIRE_THAT(src,
            IsInstantiation(_A("B<F,FF=F>"),
                _A("B<(lambda at ../../tests/t00051/t00051.cc:43:18),(lambda "
                   "at ../../tests/t00051/t00051.cc:43:27)>")));

        REQUIRE_THAT(src,
            IsDependency(_A("A"),
                _A("B<(lambda at ../../tests/t00051/t00051.cc:43:18),(lambda "
                   "at ../../tests/t00051/t00051.cc:43:27)>")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsInnerClass(j, "A", "A::custom_thread1"));
        REQUIRE(IsInnerClass(j, "A", "A::custom_thread2"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsInnerClass;
        using mermaid::IsMethod;

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsInnerClass(_A("A"), _A("A::custom_thread1")));
        REQUIRE_THAT(src, IsInnerClass(_A("A"), _A("A::custom_thread2")));

        REQUIRE_THAT(src,
            (IsMethod<Public>("custom_thread1<Function,Args...>", "void",
                "Function && f, Args &&... args")));
        REQUIRE_THAT(src,
            (IsMethod<Public>("thread", "void",
                "(lambda at ../../tests/t00051/t00051.cc:59:27) &&")));
        REQUIRE_THAT(src,
            (IsMethod<Private>("start_thread3",
                "B<(lambda at ../../tests/t00051/t00051.cc:43:18),(lambda at "
                "../../tests/t00051/t00051.cc:43:27)>")));
        REQUIRE_THAT(src,
            (IsMethod<Private>("get_function",
                "(lambda at ../../tests/t00051/t00051.cc:48:16)")));

        REQUIRE_THAT(src, IsClass(_A("B<F,FF=F>")));
        REQUIRE_THAT(src, (IsMethod<Public>("f", "void")));
        REQUIRE_THAT(src, (IsMethod<Public>("ff", "void")));

        REQUIRE_THAT(src,
            IsClass(_A(
                "B<(lambda at ../../tests/t00051/t00051.cc:43:18),(lambda at "
                "../../tests/t00051/t00051.cc:43:27)>")));

        REQUIRE_THAT(src,
            IsInstantiation(_A("B<F,FF=F>"),
                _A("B<(lambda at ../../tests/t00051/t00051.cc:43:18),(lambda "
                   "at ../../tests/t00051/t00051.cc:43:27)>")));

        REQUIRE_THAT(src,
            IsDependency(_A("A"),
                _A("B<(lambda at ../../tests/t00051/t00051.cc:43:18),(lambda "
                   "at ../../tests/t00051/t00051.cc:43:27)>")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}