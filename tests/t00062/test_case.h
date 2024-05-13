/**
 * tests/t00062/test_case.h
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

TEST_CASE("t00062")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00062");

    auto diagram = config.diagrams["t00062_class"];

    REQUIRE(diagram->name == "t00062_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00062_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(!src.contains("type-parameter-"));

        REQUIRE(IsClassTemplate(src, "A<T>"));
        REQUIRE(IsClassTemplate(src, "A<U &>"));
        REQUIRE(IsClassTemplate(src, "A<U &&>"));
        REQUIRE(IsClassTemplate(src, "A<U const&>"));
        REQUIRE(IsClassTemplate(src, "A<M C::*>"));
        REQUIRE(IsClassTemplate(src, "A<M C::* &&>"));
        REQUIRE(IsClassTemplate(src, "A<M (C::*)(Arg)>"));
        REQUIRE(IsClassTemplate(src, "A<int (C::*)(bool)>"));
        REQUIRE(IsClassTemplate(src, "A<M (C::*)(Arg) &&>"));
        REQUIRE(IsClassTemplate(src, "A<M (C::*)(Arg1,Arg2,Arg3)>"));
        REQUIRE(IsClassTemplate(src, "A<float (C::*)(int) &&>"));

        REQUIRE(IsClassTemplate(src, "A<char[N]>"));
        REQUIRE(IsClassTemplate(src, "A<char[1000]>"));

        REQUIRE(IsClassTemplate(src, "A<U(...)>"));
        REQUIRE(IsClassTemplate(src, "A<C<T>>"));
        REQUIRE(IsClassTemplate(src, "A<C<T,Args...>>"));

        REQUIRE(IsField<Public>(src, "A<U &>", "u", "U &"));
        REQUIRE(IsField<Public>(src, "A<U **>", "u", "U **"));
        REQUIRE(IsField<Public>(src, "A<U ***>", "u", "U ***"));
        REQUIRE(IsField<Public>(src, "A<U &&>", "u", "U &&"));
        REQUIRE(IsField<Public>(src, "A<const U &>", "u", "const U &"));
        REQUIRE(IsField<Public>(src, "A<C &>", "c", "C &"));
        REQUIRE(IsField<Public>(src, "A<M C::*>", "m", "M C::*"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<U &>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<U &&>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<M C::*>"));
        REQUIRE(IsInstantiation(src, "A<U &&>", "A<M C::* &&>"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<M (C::*)(Arg)>"));
        REQUIRE(
            IsInstantiation(src, "A<M (C::*)(Arg)>", "A<int (C::*)(bool)>"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<char[N]>"));
        REQUIRE(IsInstantiation(src, "A<char[N]>", "A<char[1000]>"));

        REQUIRE(IsInstantiation(src, "A<T>", "A<U(...)>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<U(...)>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<C<T>>"));
        REQUIRE(IsInstantiation(src, "A<T>", "A<C<T,Args...>>"));
    });

    /*
    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, !Contains("type-parameter-"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsClassTemplate("A", "T"));
        REQUIRE_THAT(src, IsClassTemplate("A", "U &"));
        REQUIRE_THAT(src, IsClassTemplate("A", "U &&"));
        REQUIRE_THAT(src, IsClassTemplate("A", "U const&"));
        REQUIRE_THAT(src, IsClassTemplate("A", "M C::*"));
        REQUIRE_THAT(src, IsClassTemplate("A", "M C::* &&"));
        REQUIRE_THAT(src, IsClassTemplate("A", "M (C::*)(Arg)"));
        REQUIRE_THAT(src, IsClassTemplate("A", "int (C::*)(bool)"));
        REQUIRE_THAT(src, IsClassTemplate("A", "M (C::*)(Arg) &&"));
        REQUIRE_THAT(src, IsClassTemplate("A", "M (C::*)(Arg1,Arg2,Arg3)"));
        REQUIRE_THAT(src, IsClassTemplate("A", "float (C::*)(int) &&"));

        REQUIRE_THAT(src, IsClassTemplate("A", "char[N]"));
        REQUIRE_THAT(src, IsClassTemplate("A", "char[1000]"));

        REQUIRE_THAT(src, IsClassTemplate("A", "U(...)"));
        REQUIRE_THAT(src, IsClassTemplate("A", "C<T>"));
        REQUIRE_THAT(src, IsClassTemplate("A", "C<T,Args...>"));

        REQUIRE_THAT(src, (IsField<Public>("u", "U &")));
        REQUIRE_THAT(src, (IsField<Public>("u", "U **")));
        REQUIRE_THAT(src, (IsField<Public>("u", "U ***")));
        REQUIRE_THAT(src, (IsField<Public>("u", "U &&")));
        REQUIRE_THAT(src, (IsField<Public>("u", "const U &")));
        REQUIRE_THAT(src, (IsField<Public>("c", "C &")));
        REQUIRE_THAT(src, (IsField<Public>("m", "M C::*")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<U &>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<U &&>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<M C::*>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<U &&>"), _A("A<M C::* &&>")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<M (C::*)(Arg)>")));
        REQUIRE_THAT(src,
            IsInstantiation(_A("A<M (C::*)(Arg)>"), _A("A<int (C::*)(bool)>")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<char[N]>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<char[N]>"), _A("A<char[1000]>")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<U(...)>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<U(...)>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<C<T>>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<C<T,Args...>>")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsField;

        REQUIRE_THAT(src, !Contains("type-parameter-"));

        // Check if all classes exist
        REQUIRE_THAT(src, IsClass(_A("A<T>")));
        REQUIRE_THAT(src, IsClass(_A("A<U &>")));
        REQUIRE_THAT(src, IsClass(_A("A<U &&>")));
        REQUIRE_THAT(src, IsClass(_A("A<U const&>")));
        REQUIRE_THAT(src, IsClass(_A("A<M C::*>")));
        REQUIRE_THAT(src, IsClass(_A("A<M C::* &&>")));
        REQUIRE_THAT(src, IsClass(_A("A<M (C::*)(Arg)>")));
        REQUIRE_THAT(src, IsClass(_A("A<int (C::*)(bool)>")));
        REQUIRE_THAT(src, IsClass(_A("A<M (C::*)(Arg) &&>")));
        REQUIRE_THAT(src, IsClass(_A("A<M (C::*)(Arg1,Arg2,Arg3)>")));
        REQUIRE_THAT(src, IsClass(_A("A<float (C::*)(int) &&>")));

        REQUIRE_THAT(src, IsClass(_A("A<char[N]>")));
        REQUIRE_THAT(src, IsClass(_A("A<char[1000]>")));

        REQUIRE_THAT(src, IsClass(_A("A<U(...)>")));
        REQUIRE_THAT(src, IsClass(_A("A<C<T>>")));
        REQUIRE_THAT(src, IsClass(_A("A<C<T,Args...>>")));

        REQUIRE_THAT(src, (IsField<Public>("u", "U &")));
        REQUIRE_THAT(src, (IsField<Public>("u", "U **")));
        REQUIRE_THAT(src, (IsField<Public>("u", "U ***")));
        REQUIRE_THAT(src, (IsField<Public>("u", "U &&")));
        REQUIRE_THAT(src, (IsField<Public>("u", "const U &")));
        REQUIRE_THAT(src, (IsField<Public>("c", "C &")));
        REQUIRE_THAT(src, (IsField<Public>("m", "M C::*")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<U &>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<U &&>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<M C::*>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<U &&>"), _A("A<M C::* &&>")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<M (C::*)(Arg)>")));
        REQUIRE_THAT(src,
            IsInstantiation(_A("A<M (C::*)(Arg)>"), _A("A<int (C::*)(bool)>")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<char[N]>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<char[N]>"), _A("A<char[1000]>")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<U(...)>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<U(...)>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<C<T>>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T>"), _A("A<C<T,Args...>>")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}