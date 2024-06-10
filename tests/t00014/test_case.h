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

TEST_CASE("t00014")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00014", "t00014_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(!src.contains("type-parameter-"));

        REQUIRE(IsClassTemplate(src, "A<T,P>"));
        REQUIRE(IsClassTemplate(src, "A<T,std::string>"));
        REQUIRE(IsClassTemplate(src, "A<T,std::unique_ptr<std::string>>"));
        REQUIRE(IsClassTemplate(src, "A<double,T>"));
        // TODO: Figure out how to handle the same templates with different
        // template
        //       parameter names
        //    REQUIRE(puml, !IsClassTemplate("A", "long,U"));
        REQUIRE(IsClassTemplate(src, "A<long,T>"));
        REQUIRE(IsClassTemplate(src, "A<long,bool>"));
        REQUIRE(IsClassTemplate(src, "A<double,bool>"));
        REQUIRE(IsClassTemplate(src, "A<long,float>"));
        REQUIRE(IsClassTemplate(src, "A<double,float>"));
        REQUIRE(IsClassTemplate(src, "A<bool,std::string>"));
        REQUIRE(IsClassTemplate(src, "A<std::string,std::string>"));
        REQUIRE(IsClassTemplate(src, "A<char,std::string>"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(IsClassTemplate(src, "R<T>"));

        REQUIRE(IsField<Private>(src, "R<T>", "bapair", "PairPairBA<bool>"));
        REQUIRE(IsField<Private>(src, "R<T>", "abool", "APtr<bool>"));
        REQUIRE(
            IsField<Private>(src, "R<T>", "aboolfloat", "AAPtr<bool,float>"));
        REQUIRE(IsField<Private>(src, "R<T>", "afloat", "ASharedPtr<float>"));
        REQUIRE(
            IsField<Private>(src, "R<T>", "boolstring", "A<bool,std::string>"));
        REQUIRE(
            IsField<Private>(src, "R<T>", "floatstring", "AStringPtr<float>"));
        REQUIRE(IsField<Private>(src, "R<T>", "atfloat", "AAPtr<T,float>"));

        REQUIRE(IsField<Private>(src, "R<T>", "intstring", "AIntString"));
        REQUIRE(IsField<Private>(src, "R<T>", "stringstring", "AStringString"));
        REQUIRE(
            IsField<Private>(src, "R<T>", "bstringstring", "BStringString"));

        REQUIRE(IsField<Protected>(src, "R<T>", "bs", "BVector"));

        REQUIRE(
            IsField<Public>(src, "R<T>", "cb", "SimpleCallback<ACharString>"));
#if LLVM_VERSION_MAJOR >= 16
        REQUIRE(IsField<Public>(
            src, "R<T>", "gcb", "GenericCallback<AWCharString>"));
#else
        REQUIRE(
            IsField<Public>(src, "R<T>", "gcb", "GenericCallback<R::AWCharString>"));
#endif
        REQUIRE(IsField<Public>(src, "R<T>", "vcb", "VoidCallback"));

        REQUIRE(
            !IsClassTemplate(src, "std::std::function<void(T...,int),int)>"));

        REQUIRE(IsInstantiation(src, "A<T,P>", "A<T,std::string>"));
        REQUIRE(IsInstantiation(src, "A<long,T>", "A<long,float>"));
        REQUIRE(IsInstantiation(src, "A<long,T>", "A<long,bool>"));

        REQUIRE(IsInstantiation(src, "A<T,P>", "A<long,T>"));
        //    REQUIRE(puml, !IsInstantiation(_A("A<long,T>"),
        //    _A("A<long,U>")));
        REQUIRE(IsInstantiation(src, "A<double,T>", "A<double,float>"));
        REQUIRE(IsInstantiation(src, "A<double,T>", "A<double,bool>"));
        REQUIRE(IsInstantiation(src, "A<T,P>", "A<double,T>"));
        REQUIRE(IsInstantiation(src, "A<T,P>", "A<T,std::string>"));
        REQUIRE(
            IsInstantiation(src, "A<T,std::string>", "A<bool,std::string>"));
        REQUIRE(
            IsInstantiation(src, "A<T,std::string>", "A<char,std::string>"));
        REQUIRE(
            IsInstantiation(src, "A<T,std::string>", "A<wchar_t,std::string>"));

        REQUIRE(IsInstantiation(src, "A<T,std::unique_ptr<std::string>>",
            "A<float,std::unique_ptr<std::string>>"));
        REQUIRE(IsInstantiation(
            src, "A<T,P>", "A<T,std::unique_ptr<std::string>>"));

        REQUIRE(IsAggregation<Public>(src, "R<T>", "B", "vps"));
        REQUIRE(IsAggregation<Private>(src, "R<T>", "B", "bapair"));
        REQUIRE(
            IsAggregation<Private>(src, "R<T>", "A<long,float>", "aboolfloat"));
        REQUIRE(IsAggregation<Private>(src, "R<T>", "A<long,bool>", "bapair"));
        REQUIRE(IsAggregation<Private>(
            src, "R<T>", "A<double,bool>", "aboolfloat"));
        REQUIRE(IsAggregation<Private>(src, "R<T>", "A<double,T>", "atfloat"));
        REQUIRE(
            IsAggregation<Private>(src, "R<T>", "A<long,float>", "atfloat"));
        REQUIRE(
            IsAssociation<Private>(src, "R<T>", "A<double,float>", "afloat"));
        REQUIRE(IsAggregation<Private>(
            src, "R<T>", "A<bool,std::string>", "boolstring"));
        REQUIRE(IsAggregation<Private>(src, "R<T>",
            "A<float,std::unique_ptr<std::string>>", "floatstring"));
        REQUIRE(IsDependency(src, "R<T>", "A<char,std::string>"));
        REQUIRE(IsDependency(src, "R<T>", "A<wchar_t,std::string>"));
    });
}