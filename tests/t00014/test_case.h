/**
 * tests/t00014/test_case.h
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

TEST_CASE("t00014", "[test-case][class]")
{
    auto [config, db] = load_config("t00014");

    auto diagram = config.diagrams["t00014_class"];

    REQUIRE(diagram->name == "t00014_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00014_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, !Contains("type-parameter-"));

        REQUIRE_THAT(src, IsClassTemplate("A", "T,P"));
        REQUIRE_THAT(src, IsClassTemplate("A", "T,std::string"));
        REQUIRE_THAT(
            src, IsClassTemplate("A", "T,std::unique_ptr<std::string>"));
        REQUIRE_THAT(src, IsClassTemplate("A", "double,T"));
        // TODO: Figure out how to handle the same templates with different
        // template
        //       parameter names
        //    REQUIRE_THAT(puml, !IsClassTemplate("A", "long,U"));
        REQUIRE_THAT(src, IsClassTemplate("A", "long,T"));
        REQUIRE_THAT(src, IsClassTemplate("A", "long,bool"));
        REQUIRE_THAT(src, IsClassTemplate("A", "double,bool"));
        REQUIRE_THAT(src, IsClassTemplate("A", "long,float"));
        REQUIRE_THAT(src, IsClassTemplate("A", "double,float"));
        REQUIRE_THAT(src, IsClassTemplate("A", "bool,std::string"));
        REQUIRE_THAT(src, IsClassTemplate("A", "std::string,std::string"));
        REQUIRE_THAT(src, IsClassTemplate("A", "char,std::string"));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClassTemplate("R", "T"));

        REQUIRE_THAT(src, IsField<Private>("bapair", "PairPairBA<bool>"));
        REQUIRE_THAT(src, IsField<Private>("abool", "APtr<bool>"));
        REQUIRE_THAT(src, IsField<Private>("aboolfloat", "AAPtr<bool,float>"));
        REQUIRE_THAT(src, IsField<Private>("afloat", "ASharedPtr<float>"));
        REQUIRE_THAT(
            src, IsField<Private>("boolstring", "A<bool,std::string>"));
        REQUIRE_THAT(src, IsField<Private>("floatstring", "AStringPtr<float>"));
        REQUIRE_THAT(src, IsField<Private>("atfloat", "AAPtr<T,float>"));

        REQUIRE_THAT(src, IsField<Private>("intstring", "AIntString"));
        REQUIRE_THAT(src, IsField<Private>("stringstring", "AStringString"));
        REQUIRE_THAT(src, IsField<Private>("bstringstring", "BStringString"));

        REQUIRE_THAT(src, IsField<Protected>("bs", "BVector"));

        REQUIRE_THAT(src, IsField<Public>("cb", "SimpleCallback<ACharString>"));
#if LLVM_VERSION_MAJOR >= 16
        REQUIRE_THAT(
            src, IsField<Public>("gcb", "GenericCallback<AWCharString>"));
#else
        REQUIRE_THAT(
            src, IsField<Public>("gcb", "GenericCallback<R::AWCharString>"));
#endif
        REQUIRE_THAT(src, IsField<Public>("vcb", "VoidCallback"));

        REQUIRE_THAT(
            src, !IsClassTemplate("std::std::function", "void(T...,int),int)"));

        REQUIRE_THAT(
            src, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<long,T>"), _A("A<long,float>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<long,T>"), _A("A<long,bool>")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T,P>"), _A("A<long,T>")));
        //    REQUIRE_THAT(puml, !IsInstantiation(_A("A<long,T>"),
        //    _A("A<long,U>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<double,T>"), _A("A<double,float>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<double,T>"), _A("A<double,bool>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T,P>"), _A("A<double,T>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
        REQUIRE_THAT(src,
            IsInstantiation(_A("A<T,std::string>"), _A("A<bool,std::string>")));
        REQUIRE_THAT(src,
            IsInstantiation(_A("A<T,std::string>"), _A("A<char,std::string>")));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("A<T,std::string>"), _A("A<wchar_t,std::string>")));

        REQUIRE_THAT(src,
            IsInstantiation(_A("A<T,std::unique_ptr<std::string>>"),
                _A("A<float,std::unique_ptr<std::string>>")));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("A<T,P>"), _A("A<T,std::unique_ptr<std::string>>")));

        REQUIRE_THAT(src, IsAggregation(_A("R<T>"), _A("B"), "+vps"));
        REQUIRE_THAT(src, IsAggregation(_A("R<T>"), _A("B"), "-bapair"));
        REQUIRE_THAT(
            src, IsAggregation(_A("R<T>"), _A("A<long,float>"), "-aboolfloat"));
        REQUIRE_THAT(
            src, IsAggregation(_A("R<T>"), _A("A<long,bool>"), "-bapair"));
        REQUIRE_THAT(src,
            IsAggregation(_A("R<T>"), _A("A<double,bool>"), "-aboolfloat"));
        REQUIRE_THAT(
            src, IsAggregation(_A("R<T>"), _A("A<double,T>"), "-atfloat"));
        REQUIRE_THAT(
            src, IsAggregation(_A("R<T>"), _A("A<long,float>"), "-atfloat"));
        REQUIRE_THAT(
            src, IsAssociation(_A("R<T>"), _A("A<double,float>"), "-afloat"));
        REQUIRE_THAT(src,
            IsAggregation(
                _A("R<T>"), _A("A<bool,std::string>"), "-boolstring"));
        REQUIRE_THAT(src,
            IsAggregation(_A("R<T>"),
                _A("A<float,std::unique_ptr<std::string>>"), "-floatstring"));
        REQUIRE_THAT(src, IsDependency(_A("R<T>"), _A("A<char,std::string>")));
        REQUIRE_THAT(
            src, IsDependency(_A("R<T>"), _A("A<wchar_t,std::string>")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(json::IsClass(j, "A<T,P>"));
        REQUIRE(json::IsClass(j, "A<T,std::string>"));
        REQUIRE(json::IsClass(j, "A<T,std::unique_ptr<std::string>>"));
        REQUIRE(json::IsClass(j, "A<double,T>"));
        REQUIRE(json::IsClass(j, "A<long,T>"));
        REQUIRE(json::IsClass(j, "A<long,bool>"));
        REQUIRE(json::IsClass(j, "A<double,bool>"));
        REQUIRE(json::IsClass(j, "A<long,float>"));
        REQUIRE(json::IsClass(j, "A<double,bool>"));
        REQUIRE(json::IsClass(j, "A<double,float>"));
        REQUIRE(json::IsClass(j, "A<bool,std::string>"));
        REQUIRE(json::IsClass(j, "A<std::string,std::string>"));
        REQUIRE(json::IsClass(j, "A<char,std::string>"));
        REQUIRE(json::IsClass(j, "B"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsField;

        REQUIRE_THAT(src, !Contains("type-parameter-"));

        REQUIRE_THAT(src, IsClass(_A("A<T,P>")));
        REQUIRE_THAT(src, IsClass(_A("A<T,std::string>")));
        REQUIRE_THAT(src, IsClass(_A("A<T,std::unique_ptr<std::string>>")));
        REQUIRE_THAT(src, IsClass(_A("A<double,T>")));
        // TODO: Figure out how to handle the same templates with different
        // template
        //       parameter names
        //    REQUIRE_THAT(puml, !IsClass("A", "long,U"));
        REQUIRE_THAT(src, IsClass(_A("A<long,T>")));
        REQUIRE_THAT(src, IsClass(_A("A<long,bool>")));
        REQUIRE_THAT(src, IsClass(_A("A<double,bool>")));
        REQUIRE_THAT(src, IsClass(_A("A<long,float>")));
        REQUIRE_THAT(src, IsClass(_A("A<double,float>")));
        REQUIRE_THAT(src, IsClass(_A("A<bool,std::string>")));
        REQUIRE_THAT(src, IsClass(_A("A<std::string,std::string>")));
        REQUIRE_THAT(src, IsClass(_A("A<char,std::string>")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("R<T>")));

        REQUIRE_THAT(src, IsField<Private>("bapair", "PairPairBA<bool>"));
        REQUIRE_THAT(src, IsField<Private>("abool", "APtr<bool>"));
        REQUIRE_THAT(src, IsField<Private>("aboolfloat", "AAPtr<bool,float>"));
        REQUIRE_THAT(src, IsField<Private>("afloat", "ASharedPtr<float>"));
        REQUIRE_THAT(
            src, IsField<Private>("boolstring", "A<bool,std::string>"));
        REQUIRE_THAT(src, IsField<Private>("floatstring", "AStringPtr<float>"));
        REQUIRE_THAT(src, IsField<Private>("atfloat", "AAPtr<T,float>"));

        REQUIRE_THAT(src, IsField<Private>("intstring", "AIntString"));
        REQUIRE_THAT(src, IsField<Private>("stringstring", "AStringString"));
        REQUIRE_THAT(src, IsField<Private>("bstringstring", "BStringString"));

        REQUIRE_THAT(src, IsField<Protected>("bs", "BVector"));

        REQUIRE_THAT(src, IsField<Public>("cb", "SimpleCallback<ACharString>"));
#if LLVM_VERSION_MAJOR >= 16
        REQUIRE_THAT(
            src, IsField<Public>("gcb", "GenericCallback<AWCharString>"));
#else
        REQUIRE_THAT(
            src, IsField<Public>("gcb", "GenericCallback<R::AWCharString>"));
#endif
        REQUIRE_THAT(src, IsField<Public>("vcb", "VoidCallback"));

        REQUIRE_THAT(
            src, !IsClassTemplate("std::std::function", "void(T...,int),int)"));

        REQUIRE_THAT(
            src, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<long,T>"), _A("A<long,float>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<long,T>"), _A("A<long,bool>")));

        REQUIRE_THAT(src, IsInstantiation(_A("A<T,P>"), _A("A<long,T>")));
        //    REQUIRE_THAT(puml, !IsInstantiation(_A("A<long,T>"),
        //    _A("A<long,U>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<double,T>"), _A("A<double,float>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<double,T>"), _A("A<double,bool>")));
        REQUIRE_THAT(src, IsInstantiation(_A("A<T,P>"), _A("A<double,T>")));
        REQUIRE_THAT(
            src, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
        REQUIRE_THAT(src,
            IsInstantiation(_A("A<T,std::string>"), _A("A<bool,std::string>")));
        REQUIRE_THAT(src,
            IsInstantiation(_A("A<T,std::string>"), _A("A<char,std::string>")));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("A<T,std::string>"), _A("A<wchar_t,std::string>")));

        REQUIRE_THAT(src,
            IsInstantiation(_A("A<T,std::unique_ptr<std::string>>"),
                _A("A<float,std::unique_ptr<std::string>>")));
        REQUIRE_THAT(src,
            IsInstantiation(
                _A("A<T,P>"), _A("A<T,std::unique_ptr<std::string>>")));

        REQUIRE_THAT(src, IsAggregation(_A("R<T>"), _A("B"), "+vps"));
        REQUIRE_THAT(src, IsAggregation(_A("R<T>"), _A("B"), "-bapair"));
        REQUIRE_THAT(
            src, IsAggregation(_A("R<T>"), _A("A<long,float>"), "-aboolfloat"));
        REQUIRE_THAT(
            src, IsAggregation(_A("R<T>"), _A("A<long,bool>"), "-bapair"));
        REQUIRE_THAT(src,
            IsAggregation(_A("R<T>"), _A("A<double,bool>"), "-aboolfloat"));
        REQUIRE_THAT(
            src, IsAggregation(_A("R<T>"), _A("A<double,T>"), "-atfloat"));
        REQUIRE_THAT(
            src, IsAggregation(_A("R<T>"), _A("A<long,float>"), "-atfloat"));
        REQUIRE_THAT(
            src, IsAssociation(_A("R<T>"), _A("A<double,float>"), "-afloat"));
        REQUIRE_THAT(src,
            IsAggregation(
                _A("R<T>"), _A("A<bool,std::string>"), "-boolstring"));
        REQUIRE_THAT(src,
            IsAggregation(_A("R<T>"),
                _A("A<float,std::unique_ptr<std::string>>"), "-floatstring"));
#if !defined(__APPLE__)
        // TODO(#176)
        REQUIRE_THAT(src, IsDependency(_A("R<T>"), _A("A<char,std::string>")));
        REQUIRE_THAT(
            src, IsDependency(_A("R<T>"), _A("A<wchar_t,std::string>")));
#endif

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}