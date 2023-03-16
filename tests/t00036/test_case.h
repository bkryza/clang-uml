/**
 * tests/t00036/test_case.cc
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

TEST_CASE("t00036", "[test-case][class]")
{
    auto [config, db] = load_config("t00036");

    auto diagram = config.diagrams["t00036_class"];

    REQUIRE(diagram->name == "t00036_class");
    REQUIRE(diagram->generate_packages() == true);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00036_class");

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    REQUIRE_THAT(puml, IsClassTemplate("A", "T"));
    REQUIRE_THAT(puml, IsClassTemplate("A", "int"));
    REQUIRE_THAT(puml, IsEnum(_A("E")));
    REQUIRE_THAT(puml, IsClass(_A("B")));
    REQUIRE_THAT(puml, IsClass(_A("C")));
    REQUIRE_THAT(puml, IsPackage("ns111"));
    REQUIRE_THAT(puml, IsPackage("ns22"));

    REQUIRE_THAT(puml, IsAggregation(_A("B"), _A("A<int>"), "+a_int"));

    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);

    const std::string expected_json = R"##(
{
  "elements": [
    {
      "display_name": "clanguml::t00036::ns1",
      "elements": [
        {
          "constants": [
            "blue",
            "yellow"
          ],
          "display_name": "clanguml::t00036::ns1::E",
          "id": "2144761953049158478",
          "is_nested": false,
          "name": "E",
          "namespace": "clanguml::t00036::ns1",
          "source_location": {
            "file": "../../tests/t00036/t00036.cc",
            "line": 6
          },
          "type": "enum"
        },
        {
          "display_name": "clanguml::t00036::ns1::ns11",
          "elements": [
            {
              "bases": [],
              "display_name": "clanguml::t00036::ns1::ns11::A<T>",
              "id": "571573305652194946",
              "is_abstract": false,
              "is_nested": false,
              "is_struct": true,
              "is_template": false,
              "is_union": false,
              "members": [
                {
                  "access": "public",
                  "is_static": false,
                  "name": "a",
                  "source_location": {
                    "file": "../../tests/t00036/t00036.cc",
                    "line": 11
                  },
                  "type": "T"
                }
              ],
              "methods": [],
              "name": "A",
              "namespace": "clanguml::t00036::ns1::ns11",
              "source_location": {
                "file": "../../tests/t00036/t00036.cc",
                "line": 10
              },
              "template_parameters": [
                {
                  "is_variadic": false,
                  "kind": "template_type",
                  "name": "T"
                }
              ],
              "type": "class"
            },
            {
              "display_name": "clanguml::t00036::ns1::ns11::ns111",
              "elements": [
                {
                  "bases": [],
                  "display_name": "clanguml::t00036::ns1::ns11::ns111::B",
                  "id": "1964031933563607376",
                  "is_abstract": false,
                  "is_nested": false,
                  "is_struct": true,
                  "is_template": false,
                  "is_union": false,
                  "members": [
                    {
                      "access": "public",
                      "is_static": false,
                      "name": "a_int",
                      "source_location": {
                        "file": "../../tests/t00036/t00036.cc",
                        "line": 17
                      },
                      "type": "A<int>"
                    }
                  ],
                  "methods": [],
                  "name": "B",
                  "namespace": "clanguml::t00036::ns1::ns11::ns111",
                  "source_location": {
                    "file": "../../tests/t00036/t00036.cc",
                    "line": 16
                  },
                  "template_parameters": [],
                  "type": "class"
                }
              ],
              "name": "ns111",
              "type": "namespace"
            },
            {
              "bases": [],
              "display_name": "clanguml::t00036::ns1::ns11::A<int>",
              "id": "1832710427462319797",
              "is_abstract": false,
              "is_nested": false,
              "is_struct": false,
              "is_template": false,
              "is_union": false,
              "members": [],
              "methods": [],
              "name": "A",
              "namespace": "clanguml::t00036::ns1::ns11",
              "template_parameters": [
                {
                  "is_variadic": false,
                  "kind": "argument",
                  "type": "int"
                }
              ],
              "type": "class"
            }
          ],
          "name": "ns11",
          "type": "namespace"
        }
      ],
      "name": "ns1",
      "type": "namespace"
    },
    {
      "display_name": "clanguml::t00036::ns2",
      "elements": [
        {
          "display_name": "clanguml::t00036::ns2::ns22",
          "elements": [
            {
              "bases": [],
              "display_name": "clanguml::t00036::ns2::ns22::C",
              "id": "2038956882066165590",
              "is_abstract": false,
              "is_nested": false,
              "is_struct": true,
              "is_template": false,
              "is_union": false,
              "members": [],
              "methods": [],
              "name": "C",
              "namespace": "clanguml::t00036::ns2::ns22",
              "source_location": {
                "file": "../../tests/t00036/t00036.cc",
                "line": 28
              },
              "template_parameters": [],
              "type": "class"
            }
          ],
          "name": "ns22",
          "type": "namespace"
        }
      ],
      "name": "ns2",
      "type": "namespace"
    }
  ],
  "relationships": [
    {
      "access": "public",
      "destination": "1832710427462319797",
      "label": "a_int",
      "source": "1964031933563607376",
      "type": "aggregation"
    },
    {
      "access": "public",
      "destination": "571573305652194946",
      "source": "1832710427462319797",
      "type": "instantiation"
    }
  ]
}
)##";

    auto j = generate_class_json(diagram, *model);

    REQUIRE(j == nlohmann::json::parse(expected_json));

    save_json(config.output_directory() + "/" + diagram->name + ".json", j);
}
