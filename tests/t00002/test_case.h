/**
 * tests/t00002/test_case.cc
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

TEST_CASE("t00002", "[test-case][class]")
{
    auto [config, db] = load_config("t00002");

    auto diagram = config.diagrams["t00002_class"];

    REQUIRE(diagram->name == "t00002_class");

    REQUIRE(diagram->include().namespaces.size() == 1);
    REQUIRE_THAT(diagram->include().namespaces,
        VectorContains(
            clanguml::common::model::namespace_{"clanguml::t00002"}));

    REQUIRE(diagram->exclude().namespaces.size() == 0);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00002_class");

    REQUIRE(model->should_include({"clanguml", "t00002"}, "A"));
    REQUIRE(!model->should_include({"std"}, "vector"));

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));
    REQUIRE_THAT(puml, IsAbstractClass(_A("A")));
    REQUIRE_THAT(puml, IsClass(_A("B")));
    REQUIRE_THAT(puml, IsClass(_A("C")));
    REQUIRE_THAT(puml, IsClass(_A("D")));
    REQUIRE_THAT(puml, IsBaseClass(_A("A"), _A("B")));
    REQUIRE_THAT(puml, IsBaseClass(_A("A"), _A("C")));
    REQUIRE_THAT(puml, IsBaseClass(_A("B"), _A("D")));
    REQUIRE_THAT(puml, IsBaseClass(_A("C"), _A("D")));
    REQUIRE_THAT(puml, (IsMethod<Public, Abstract>("foo_a")));
    REQUIRE_THAT(puml, (IsMethod<Public, Abstract>("foo_c")));

    REQUIRE_THAT(puml, IsAssociation(_A("D"), _A("A"), "-as"));

    REQUIRE_THAT(puml, HasNote(_A("A"), "left", "This is class A"));
    REQUIRE_THAT(puml, HasNote(_A("B"), "top", "This is class B"));

    REQUIRE_THAT(puml,
        HasLink(_A("A"),
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t00002/t00002.cc#L7",
                clanguml::util::get_git_commit()),
            "This is class A"));

    REQUIRE_THAT(puml,
        HasLink(_A("B"),
            fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                        "t00002/t00002.cc#L16",
                clanguml::util::get_git_commit()),
            "This is class B"));

    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);

    const std::string expected_json = R"##(
{
  "elements": [
    {
      "bases": [],
      "comment": {
        "brief": [
          " This is class A\n"
        ],
        "formatted": "\\brief This is class A",
        "paragraph": [
          " \n"
        ],
        "raw": "/// \\brief This is class A",
        "text": "\n \n"
      },
      "display_name": "clanguml::t00002::A",
      "id": "987634239855407298",
      "is_abstract": true,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [
        {
          "access": "public",
          "comment": {
            "formatted": "Abstract foo_a",
            "paragraph": [
              " Abstract foo_a\n"
            ],
            "raw": "/// Abstract foo_a",
            "text": "\n Abstract foo_a\n"
          },
          "is_const": false,
          "is_defaulted": false,
          "is_implicit": false,
          "is_pure_virtual": true,
          "is_static": false,
          "is_virtual": true,
          "name": "foo_a",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "public",
          "comment": {
            "formatted": "Abstract foo_c",
            "paragraph": [
              " Abstract foo_c\n"
            ],
            "raw": "/// Abstract foo_c",
            "text": "\n Abstract foo_c\n"
          },
          "is_const": false,
          "is_defaulted": false,
          "is_implicit": false,
          "is_pure_virtual": true,
          "is_static": false,
          "is_virtual": true,
          "name": "foo_c",
          "parameters": [],
          "type": "void"
        }
      ],
      "name": "A",
      "namespace": "clanguml::t00002",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00002/t00002.cc",
        "line": 7
      },
      "template_parameters": [],
      "type": "class"
    },
    {
      "bases": [
        {
          "access": "public",
          "id": "987634239855407298",
          "is_virtual": false,
          "name": "clanguml::t00002::A"
        }
      ],
      "comment": {
        "brief": [
          " This is class B\n"
        ],
        "formatted": "\\brief This is class B",
        "paragraph": [
          " \n"
        ],
        "raw": "/// \\brief This is class B",
        "text": "\n \n"
      },
      "display_name": "clanguml::t00002::B",
      "id": "594234458687375950",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [
        {
          "access": "public",
          "is_const": false,
          "is_defaulted": false,
          "is_implicit": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": true,
          "name": "foo_a",
          "parameters": [],
          "type": "void"
        }
      ],
      "name": "B",
      "namespace": "clanguml::t00002",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00002/t00002.cc",
        "line": 16
      },
      "template_parameters": [],
      "type": "class"
    },
    {
      "bases": [
        {
          "access": "public",
          "id": "987634239855407298",
          "is_virtual": false,
          "name": "clanguml::t00002::A"
        }
      ],
      "comment": {
        "brief": [
          " This is class C - class C has a long comment\n"
        ],
        "formatted": "@brief This is class C - class C has a long comment\n\nVivamus integer non suscipit taciti mus etiam at primis tempor sagittis sit,\neuismod libero facilisi aptent elementum felis blandit cursus gravida sociis\nerat ante, eleifend lectus nullam dapibus netus feugiat curae curabitur est\nad.",
        "paragraph": [
          " \n",
          " Vivamus integer non suscipit taciti mus etiam at primis tempor sagittis sit,\n euismod libero facilisi aptent elementum felis blandit cursus gravida sociis\n erat ante, eleifend lectus nullam dapibus netus feugiat curae curabitur est\n ad.\n"
        ],
        "raw": "/// @brief This is class C - class C has a long comment\n///\n/// Vivamus integer non suscipit taciti mus etiam at primis tempor sagittis sit,\n/// euismod libero facilisi aptent elementum felis blandit cursus gravida sociis\n/// erat ante, eleifend lectus nullam dapibus netus feugiat curae curabitur est\n/// ad.",
        "text": "\n \n\n Vivamus integer non suscipit taciti mus etiam at primis tempor sagittis sit,\n euismod libero facilisi aptent elementum felis blandit cursus gravida sociis\n erat ante, eleifend lectus nullam dapibus netus feugiat curae curabitur est\n ad.\n"
      },
      "display_name": "clanguml::t00002::C",
      "id": "1142499429598587507",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [
        {
          "access": "public",
          "comment": {
            "formatted": "Do nothing unless override is provided",
            "paragraph": [
              " Do nothing unless override is provided\n"
            ],
            "raw": "/// Do nothing unless override is provided",
            "text": "\n Do nothing unless override is provided\n"
          },
          "is_const": false,
          "is_defaulted": false,
          "is_implicit": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": true,
          "name": "foo_c",
          "parameters": [],
          "type": "void"
        }
      ],
      "name": "C",
      "namespace": "clanguml::t00002",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00002/t00002.cc",
        "line": 27
      },
      "template_parameters": [],
      "type": "class"
    },
    {
      "bases": [
        {
          "access": "public",
          "id": "594234458687375950",
          "is_virtual": false,
          "name": "clanguml::t00002::B"
        },
        {
          "access": "public",
          "id": "1142499429598587507",
          "is_virtual": false,
          "name": "clanguml::t00002::C"
        }
      ],
      "comment": {
        "formatted": "This is class D\nwhich is a little like B\nand a little like C",
        "paragraph": [
          " This is class D\n which is a little like B\n and a little like C\n"
        ],
        "raw": "/// This is class D\n/// which is a little like B\n/// and a little like C",
        "text": "\n This is class D\n which is a little like B\n and a little like C\n"
      },
      "display_name": "clanguml::t00002::D",
      "id": "60950494980414724",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "private",
          "comment": {
            "formatted": "All the A pointers",
            "paragraph": [
              " All the A pointers\n"
            ],
            "raw": "/// All the A pointers",
            "text": "\n All the A pointers\n"
          },
          "is_static": false,
          "name": "as",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00002/t00002.cc",
            "line": 58
          },
          "type": "std::vector<A *>"
        }
      ],
      "methods": [
        {
          "access": "public",
          "comment": {
            "formatted": "\n Forward foo_a\n     ",
            "paragraph": [
              " Forward foo_a\n"
            ],
            "raw": "/**\n     * Forward foo_a\n     */",
            "text": "\n Forward foo_a\n"
          },
          "is_const": false,
          "is_defaulted": false,
          "is_implicit": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": true,
          "name": "foo_a",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "public",
          "comment": {
            "formatted": "\n Forward foo_c\n     ",
            "paragraph": [
              " Forward foo_c\n"
            ],
            "raw": "/**\n     * Forward foo_c\n     */",
            "text": "\n Forward foo_c\n"
          },
          "is_const": false,
          "is_defaulted": false,
          "is_implicit": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": true,
          "name": "foo_c",
          "parameters": [],
          "type": "void"
        }
      ],
      "name": "D",
      "namespace": "clanguml::t00002",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00002/t00002.cc",
        "line": 36
      },
      "template_parameters": [],
      "type": "class"
    },
    {
      "bases": [
        {
          "access": "public",
          "id": "594234458687375950",
          "is_virtual": true,
          "name": "clanguml::t00002::B"
        },
        {
          "access": "public",
          "id": "1142499429598587507",
          "is_virtual": true,
          "name": "clanguml::t00002::C"
        }
      ],
      "display_name": "clanguml::t00002::E",
      "id": "2237886670308966220",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "private",
          "comment": {
            "formatted": "All the A pointers",
            "paragraph": [
              " All the A pointers\n"
            ],
            "raw": "/// All the A pointers",
            "text": "\n All the A pointers\n"
          },
          "is_static": false,
          "name": "as",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00002/t00002.cc",
            "line": 83
          },
          "type": "std::vector<A *>"
        }
      ],
      "methods": [
        {
          "access": "public",
          "comment": {
            "formatted": "\n Forward foo_a",
            "paragraph": [
              " Forward foo_a\n"
            ],
            "raw": "///\n    /// Forward foo_a\n    ///",
            "text": "\n Forward foo_a\n"
          },
          "is_const": false,
          "is_defaulted": false,
          "is_implicit": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": true,
          "name": "foo_a",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "public",
          "comment": {
            "formatted": "\n Forward foo_c",
            "paragraph": [
              " Forward foo_c\n"
            ],
            "raw": "///\n    /// Forward foo_c\n    ///",
            "text": "\n Forward foo_c\n"
          },
          "is_const": false,
          "is_defaulted": false,
          "is_implicit": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": true,
          "name": "foo_c",
          "parameters": [],
          "type": "void"
        }
      ],
      "name": "E",
      "namespace": "clanguml::t00002",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00002/t00002.cc",
        "line": 61
      },
      "template_parameters": [],
      "type": "class"
    }
  ],
  "relationships": [
    {
      "access": "public",
      "destination": "987634239855407298",
      "source": "594234458687375950",
      "type": "extension"
    },
    {
      "access": "public",
      "destination": "987634239855407298",
      "source": "1142499429598587507",
      "type": "extension"
    },
    {
      "access": "private",
      "destination": "987634239855407298",
      "label": "as",
      "source": "60950494980414724",
      "type": "association"
    },
    {
      "access": "public",
      "destination": "594234458687375950",
      "source": "60950494980414724",
      "type": "extension"
    },
    {
      "access": "public",
      "destination": "1142499429598587507",
      "source": "60950494980414724",
      "type": "extension"
    },
    {
      "access": "private",
      "destination": "987634239855407298",
      "label": "as",
      "source": "2237886670308966220",
      "type": "association"
    },
    {
      "access": "public",
      "destination": "594234458687375950",
      "source": "2237886670308966220",
      "type": "extension"
    },
    {
      "access": "public",
      "destination": "1142499429598587507",
      "source": "2237886670308966220",
      "type": "extension"
    }
  ]
}
)##";
    auto j = generate_class_json(diagram, *model);

    //    REQUIRE(j == nlohmann::json::parse(expected_json));

    save_json(config.output_directory() + "/" + diagram->name + ".json", j);
}