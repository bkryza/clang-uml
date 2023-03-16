/**
 * tests/t00014/test_case.h
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

TEST_CASE("t00014", "[test-case][class]")
{
    auto [config, db] = load_config("t00014");

    auto diagram = config.diagrams["t00014_class"];

    REQUIRE(diagram->name == "t00014_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00014_class");
    REQUIRE(model->should_include("clanguml::t00014::B"));

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "T,P"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "T,std::string"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "T,std::unique_ptr<std::string>"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "double,T"));
    // TODO: Figure out how to handle the same templates with different template
    //       parameter names
    //    REQUIRE_THAT(puml, !IsClassTemplate("A", "long,U"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "long,T"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "long,bool"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "double,bool"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "long,float"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "double,float"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "bool,std::string"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "std::string,std::string"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "char,std::string"));
    REQUIRE_THAT(puml, IsClass(_A("B")));

    REQUIRE_THAT(puml, IsField<Private>("bapair", "PairPairBA<bool>"));
    REQUIRE_THAT(puml, IsField<Private>("abool", "APtr<bool>"));
    REQUIRE_THAT(puml, IsField<Private>("aboolfloat", "AAPtr<bool,float>"));
    REQUIRE_THAT(puml, IsField<Private>("afloat", "ASharedPtr<float>"));
    REQUIRE_THAT(puml, IsField<Private>("boolstring", "A<bool,std::string>"));
    REQUIRE_THAT(puml, IsField<Private>("floatstring", "AStringPtr<float>"));
    REQUIRE_THAT(puml, IsField<Private>("intstring", "AIntString"));
    REQUIRE_THAT(puml, IsField<Private>("stringstring", "AStringString"));
    REQUIRE_THAT(puml, IsField<Private>("bstringstring", "BStringString"));

    REQUIRE_THAT(puml, IsField<Protected>("bs", "BVector"));

    REQUIRE_THAT(puml, IsField<Public>("cb", "SimpleCallback<ACharString>"));
    REQUIRE_THAT(
        puml, IsField<Public>("gcb", "GenericCallback<R::AWCharString>"));
    REQUIRE_THAT(puml, IsField<Public>("vcb", "VoidCallback"));

    REQUIRE_THAT(
        puml, !IsClassTemplate("std::std::function", "void(T...,int),int)"));

    REQUIRE_THAT(puml, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<long,T>"), _A("A<long,float>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<long,T>"), _A("A<long,bool>")));

    REQUIRE_THAT(puml, IsInstantiation(_A("A<T,P>"), _A("A<long,T>")));
    //    REQUIRE_THAT(puml, !IsInstantiation(_A("A<long,T>"),
    //    _A("A<long,U>")));
    REQUIRE_THAT(
        puml, IsInstantiation(_A("A<double,T>"), _A("A<double,float>")));
    REQUIRE_THAT(
        puml, IsInstantiation(_A("A<double,T>"), _A("A<double,bool>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<T,P>"), _A("A<double,T>")));
    REQUIRE_THAT(puml, IsInstantiation(_A("A<T,P>"), _A("A<T,std::string>")));
    REQUIRE_THAT(puml,
        IsInstantiation(_A("A<T,std::string>"), _A("A<bool,std::string>")));
    REQUIRE_THAT(puml,
        IsInstantiation(_A("A<T,std::string>"), _A("A<char,std::string>")));
    REQUIRE_THAT(puml,
        IsInstantiation(_A("A<T,std::string>"), _A("A<wchar_t,std::string>")));

    REQUIRE_THAT(puml,
        IsInstantiation(_A("A<T,std::unique_ptr<std::string>>"),
            _A("A<float,std::unique_ptr<std::string>>")));
    REQUIRE_THAT(puml,
        IsInstantiation(_A("A<T,P>"), _A("A<T,std::unique_ptr<std::string>>")));

    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("B"), "+vps"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("B"), "-bapair"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("R"), _A("A<long,float>"), "-aboolfloat"));
    REQUIRE_THAT(puml, IsAggregation(_A("R"), _A("A<long,bool>"), "-bapair"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("R"), _A("A<double,bool>"), "-aboolfloat"));
    REQUIRE_THAT(
        puml, IsAssociation(_A("R"), _A("A<double,float>"), "-afloat"));
    REQUIRE_THAT(
        puml, IsAggregation(_A("R"), _A("A<bool,std::string>"), "-boolstring"));
    REQUIRE_THAT(puml,
        IsAggregation(_A("R"), _A("A<float,std::unique_ptr<std::string>>"),
            "-floatstring"));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("A<char,std::string>")));
    REQUIRE_THAT(puml, IsDependency(_A("R"), _A("A<wchar_t,std::string>")));

    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);

    std::string expected_json = R"##(
{
  "elements": [
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<T,P>",
      "id": "765890579167335652",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": true,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "public",
          "is_static": false,
          "name": "t",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 22
          },
          "type": "T"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "p",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 23
          },
          "type": "P"
        }
      ],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "source_location": {
        "file": "../../tests/t00014/t00014.cc",
        "line": 21
      },
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "template_type",
          "name": "T"
        },
        {
          "is_variadic": false,
          "kind": "template_type",
          "name": "P"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::B",
      "id": "934136012292043506",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": true,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "public",
          "is_static": false,
          "name": "value",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 27
          },
          "type": "std::string"
        }
      ],
      "methods": [],
      "name": "B",
      "namespace": "clanguml::t00014",
      "source_location": {
        "file": "../../tests/t00014/t00014.cc",
        "line": 26
      },
      "template_parameters": [],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<T,std::string>",
      "id": "2186387853087008570",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "T"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::string"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<T,std::unique_ptr<std::string>>",
      "id": "947292733740993297",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "T"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::unique_ptr"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<long,T>",
      "id": "1700006390494465667",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "long"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "T"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<double,T>",
      "id": "2017665567517853203",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "double"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "T"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<long,U>",
      "id": "906557320263235873",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "long"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "U"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<long,bool>",
      "id": "378898020828430636",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "long"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "bool"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<double,bool>",
      "id": "2082013375525130414",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "double"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "bool"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<long,float>",
      "id": "51978493292659230",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "long"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "float"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<double,float>",
      "id": "197769253782961588",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "double"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "float"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<bool,std::string>",
      "id": "895940711566401184",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "bool"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::string"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<float,std::unique_ptr<std::string>>",
      "id": "1751732625010742161",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "float"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::unique_ptr"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<int,std::string>",
      "id": "887121441210847583",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "int"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::string"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<std::string,std::string>",
      "id": "1119452495635561975",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::string"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::string"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<char,std::string>",
      "id": "640294848489463071",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "char"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::string"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::A<wchar_t,std::string>",
      "id": "139599686499155694",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00014",
      "template_parameters": [
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "wchar_t"
        },
        {
          "is_variadic": false,
          "kind": "argument",
          "type": "std::string"
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00014::R",
      "id": "1192822659863756768",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "private",
          "is_static": false,
          "name": "bapair",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 58
          },
          "type": "PairPairBA<bool>"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "abool",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 60
          },
          "type": "APtr<bool>"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "aboolfloat",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 61
          },
          "type": "AAPtr<bool,float>"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "afloat",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 62
          },
          "type": "ASharedPtr<float>"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "boolstring",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 63
          },
          "type": "A<bool,std::string>"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "floatstring",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 64
          },
          "type": "AStringPtr<float>"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "intstring",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 65
          },
          "type": "clanguml::t00014::AIntString"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "stringstring",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 66
          },
          "type": "clanguml::t00014::AStringString"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "bstringstring",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 67
          },
          "type": "clanguml::t00014::BStringString"
        },
        {
          "access": "protected",
          "is_static": false,
          "name": "bs",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 70
          },
          "type": "clanguml::t00014::BVector"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "bs2",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 73
          },
          "type": "clanguml::t00014::BVector2"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "cb",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 74
          },
          "type": "SimpleCallback<clanguml::t00014::ACharString>"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "gcb",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 75
          },
          "type": "GenericCallback<clanguml::t00014::R::AWCharString>"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "vcb",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 76
          },
          "type": "clanguml::t00014::VoidCallback"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "vps",
          "source_location": {
            "file": "../../tests/t00014/t00014.cc",
            "line": 77
          },
          "type": "VectorPtr<clanguml::t00014::B>"
        }
      ],
      "methods": [],
      "name": "R",
      "namespace": "clanguml::t00014",
      "source_location": {
        "file": "../../tests/t00014/t00014.cc",
        "line": 55
      },
      "template_parameters": [],
      "type": "class"
    }
  ],
  "relationships": [
    {
      "access": "public",
      "destination": "765890579167335652",
      "source": "2186387853087008570",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "765890579167335652",
      "source": "947292733740993297",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "765890579167335652",
      "source": "1700006390494465667",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "765890579167335652",
      "source": "2017665567517853203",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "1700006390494465667",
      "source": "906557320263235873",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "1700006390494465667",
      "source": "378898020828430636",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "2017665567517853203",
      "source": "2082013375525130414",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "1700006390494465667",
      "source": "51978493292659230",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "2017665567517853203",
      "source": "197769253782961588",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "2186387853087008570",
      "source": "895940711566401184",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "947292733740993297",
      "source": "1751732625010742161",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "2186387853087008570",
      "source": "887121441210847583",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "2186387853087008570",
      "source": "1119452495635561975",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "2186387853087008570",
      "source": "640294848489463071",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "2186387853087008570",
      "source": "139599686499155694",
      "type": "instantiation"
    },
    {
      "access": "public",
      "destination": "378898020828430636",
      "source": "1192822659863756768",
      "type": "dependency"
    },
    {
      "access": "private",
      "destination": "934136012292043506",
      "label": "bapair",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "private",
      "destination": "378898020828430636",
      "label": "bapair",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "public",
      "destination": "2082013375525130414",
      "source": "1192822659863756768",
      "type": "dependency"
    },
    {
      "access": "private",
      "destination": "2082013375525130414",
      "label": "abool",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "public",
      "destination": "51978493292659230",
      "source": "1192822659863756768",
      "type": "dependency"
    },
    {
      "access": "private",
      "destination": "2082013375525130414",
      "label": "aboolfloat",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "private",
      "destination": "51978493292659230",
      "label": "aboolfloat",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "public",
      "destination": "197769253782961588",
      "source": "1192822659863756768",
      "type": "dependency"
    },
    {
      "access": "private",
      "destination": "197769253782961588",
      "label": "afloat",
      "source": "1192822659863756768",
      "type": "association"
    },
    {
      "access": "private",
      "destination": "895940711566401184",
      "label": "boolstring",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "private",
      "destination": "1751732625010742161",
      "label": "floatstring",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "private",
      "destination": "887121441210847583",
      "label": "intstring",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "private",
      "destination": "1119452495635561975",
      "label": "stringstring",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "private",
      "destination": "1119452495635561975",
      "label": "bstringstring",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "protected",
      "destination": "934136012292043506",
      "label": "bs",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "public",
      "destination": "934136012292043506",
      "label": "bs2",
      "source": "1192822659863756768",
      "type": "aggregation"
    },
    {
      "access": "public",
      "destination": "640294848489463071",
      "source": "1192822659863756768",
      "type": "dependency"
    },
    {
      "access": "public",
      "destination": "139599686499155694",
      "source": "1192822659863756768",
      "type": "dependency"
    },
    {
      "access": "public",
      "destination": "934136012292043506",
      "label": "vps",
      "source": "1192822659863756768",
      "type": "aggregation"
    }
  ]
}
)##";
    auto j = generate_class_json(diagram, *model);

    REQUIRE(j == nlohmann::json::parse(expected_json));

    save_json(config.output_directory() + "/" + diagram->name + ".json", j);
}
