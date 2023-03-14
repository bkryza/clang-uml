/**
 * tests/t00056/test_case.h
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

TEST_CASE("t00056", "[test-case][class]")
{
    auto [config, db] = load_config("t00056");

    auto diagram = config.diagrams["t00056_class"];

    REQUIRE(diagram->name == "t00056_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00056_class");

    auto puml = generate_class_puml(diagram, *model);
    AliasMatcher _A(puml);

    REQUIRE_THAT(puml, StartsWith("@startuml"));
    REQUIRE_THAT(puml, EndsWith("@enduml\n"));

    // Check if all classes exist
    REQUIRE_THAT(puml, IsConcept(_A("greater_than_simple<T,L>")));
    REQUIRE_THAT(puml, IsConcept(_A("greater_than_with_requires<T,P>")));
    REQUIRE_THAT(puml, IsConcept(_A("max_four_bytes<T>")));
    REQUIRE_THAT(puml, IsConcept(_A("iterable<T>")));
    REQUIRE_THAT(puml, IsConcept(_A("has_value_type<T>")));
    REQUIRE_THAT(puml, IsConcept(_A("convertible_to_string<T>")));
    REQUIRE_THAT(puml, IsConcept(_A("iterable_with_value_type<T>")));
    REQUIRE_THAT(puml, IsConcept(_A("iterable_or_small_value_type<T>")));

    REQUIRE_THAT(puml,
        IsConceptRequirement(
            _A("greater_than_with_requires<T,P>"), "sizeof (l) > sizeof (r)"));

    REQUIRE_THAT(
        puml, IsConceptRequirement(_A("iterable<T>"), "container.begin()"));
    REQUIRE_THAT(
        puml, IsConceptRequirement(_A("iterable<T>"), "container.end()"));

#ifdef _MSC_VER
    REQUIRE_THAT(puml,
        IsConceptRequirement(
            _A("convertible_to_string<T>"), "std::string({s})"));
#else
    REQUIRE_THAT(puml,
        IsConceptRequirement(_A("convertible_to_string<T>"), "std::string{s}"));
#endif
    REQUIRE_THAT(puml,
        IsConceptRequirement(
            _A("convertible_to_string<T>"), "{std::to_string(s)} noexcept"));
    REQUIRE_THAT(puml,
        IsConceptRequirement(_A("convertible_to_string<T>"),
            "{std::to_string(s)} -> std::same_as<std::string>"));

    // Check if class templates exist
    REQUIRE_THAT(puml, IsClassTemplate("A", "max_four_bytes T"));
    REQUIRE_THAT(puml, IsClassTemplate("B", "T"));
    REQUIRE_THAT(puml, IsClassTemplate("C", "convertible_to_string T"));
    REQUIRE_THAT(
        puml, IsClassTemplate("D", "iterable T1,T2,iterable T3,T4,T5"));
    REQUIRE_THAT(puml, IsClassTemplate("E", "T1,T2,T3"));
    REQUIRE_THAT(puml, IsClassTemplate("F", "T1,T2,T3"));

    // Check if all relationships exist
    REQUIRE_THAT(puml,
        IsConstraint(_A("A<max_four_bytes T>"), _A("max_four_bytes<T>"), "T"));

    REQUIRE_THAT(puml,
        IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
            _A("max_four_bytes<T>"), "T2"));
    REQUIRE_THAT(puml,
        IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
            _A("max_four_bytes<T>"), "T5"));
    REQUIRE_THAT(puml,
        IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
            _A("iterable<T>"), "T1"));
    REQUIRE_THAT(puml,
        IsConstraint(_A("D<iterable T1,T2,iterable T3,T4,T5>"),
            _A("iterable<T>"), "T3"));

    REQUIRE_THAT(puml,
        IsConstraint(
            _A("iterable_with_value_type<T>"), _A("has_value_type<T>"), "T"));

    REQUIRE_THAT(puml,
        IsConstraint(_A("iterable_or_small_value_type<T>"),
            _A("max_four_bytes<T>"), "T"));
    REQUIRE_THAT(puml,
        IsConstraint(_A("iterable_or_small_value_type<T>"),
            _A("iterable_with_value_type<T>"), "T"));

    REQUIRE_THAT(puml,
        IsConstraint(
            _A("E<T1,T2,T3>"), _A("greater_than_with_requires<T,P>"), "T1,T3"));

    REQUIRE_THAT(puml,
        IsConstraint(
            _A("F<T1,T2,T3>"), _A("greater_than_simple<T,L>"), "T1,T3"));

    save_puml(config.output_directory() + "/" + diagram->name + ".puml", puml);

    const std::string expected_json = R"##(
{
  "elements": [
    {
      "display_name": "clanguml::t00056::greater_than_simple<T,L>",
      "id": "902541696362244204",
      "name": "greater_than_simple",
      "namespace": "clanguml::t00056",
      "parameters": [],
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 7
      },
      "statements": [],
      "type": "concept"
    },
    {
      "display_name": "clanguml::t00056::greater_than_with_requires<T,P>",
      "id": "1830716585637735576",
      "name": "greater_than_with_requires",
      "namespace": "clanguml::t00056",
      "parameters": [
        {
          "name": "clanguml::t00056::l",
          "type": "T"
        },
        {
          "name": "clanguml::t00056::r",
          "type": "P"
        }
      ],
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 10
      },
      "statements": [
        "sizeof (l) > sizeof (r)"
      ],
      "type": "concept"
    },
    {
      "display_name": "clanguml::t00056::max_four_bytes<T>",
      "id": "385255522691733325",
      "name": "max_four_bytes",
      "namespace": "clanguml::t00056",
      "parameters": [],
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 15
      },
      "statements": [],
      "type": "concept"
    },
    {
      "display_name": "clanguml::t00056::iterable<T>",
      "id": "392540961352249242",
      "name": "iterable",
      "namespace": "clanguml::t00056",
      "parameters": [
        {
          "name": "clanguml::t00056::container",
          "type": "T"
        }
      ],
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 19
      },
      "statements": [
        "container.begin()",
        "container.end()"
      ],
      "type": "concept"
    },
    {
      "display_name": "clanguml::t00056::has_value_type<T>",
      "id": "1850394311226276678",
      "name": "has_value_type",
      "namespace": "clanguml::t00056",
      "parameters": [],
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 26
      },
      "statements": [
        "typename T::value_type"
      ],
      "type": "concept"
    },
    {
      "display_name": "clanguml::t00056::convertible_to_string<T>",
      "id": "137304962071054497",
      "name": "convertible_to_string",
      "namespace": "clanguml::t00056",
      "parameters": [
        {
          "name": "clanguml::t00056::s",
          "type": "T"
        }
      ],
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 29
      },
      "statements": [
        "std::string{s}",
        "{std::to_string(s)} noexcept",
        "{std::to_string(s)} -> std::same_as<std::string>"
      ],
      "type": "concept"
    },
    {
      "display_name": "clanguml::t00056::iterable_with_value_type<T>",
      "id": "1043398062146751019",
      "name": "iterable_with_value_type",
      "namespace": "clanguml::t00056",
      "parameters": [],
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 45
      },
      "statements": [],
      "type": "concept"
    },
    {
      "display_name": "clanguml::t00056::iterable_or_small_value_type<T>",
      "id": "866345615551223718",
      "name": "iterable_or_small_value_type",
      "namespace": "clanguml::t00056",
      "parameters": [],
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 48
      },
      "statements": [],
      "type": "concept"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00056::A<clanguml::t00056::max_four_bytes T>",
      "id": "1418333499545421661",
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
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 53
          },
          "type": "T"
        }
      ],
      "methods": [],
      "name": "A",
      "namespace": "clanguml::t00056",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 52
      },
      "template_parameters": [
        {
          "concept_constraint": "clanguml::t00056::max_four_bytes",
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T",
          "type": ""
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00056::B<T>",
      "id": "1814355496814977880",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": true,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "public",
          "is_static": false,
          "name": "b",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 60
          },
          "type": "T"
        }
      ],
      "methods": [],
      "name": "B",
      "namespace": "clanguml::t00056",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 59
      },
      "template_parameters": [
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T",
          "type": ""
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00056::C<clanguml::t00056::convertible_to_string T>",
      "id": "1512618198241549089",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": true,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "public",
          "is_static": false,
          "name": "c",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 70
          },
          "type": "T"
        }
      ],
      "methods": [],
      "name": "C",
      "namespace": "clanguml::t00056",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 69
      },
      "template_parameters": [
        {
          "concept_constraint": "clanguml::t00056::convertible_to_string",
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T",
          "type": ""
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00056::D<clanguml::t00056::iterable T1,T2,clanguml::t00056::iterable T3,T4,T5>",
      "id": "1635109601630198093",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": true,
      "is_template": false,
      "is_union": false,
      "members": [],
      "methods": [],
      "name": "D",
      "namespace": "clanguml::t00056",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 75
      },
      "template_parameters": [
        {
          "concept_constraint": "clanguml::t00056::iterable",
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T1",
          "type": ""
        },
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T2",
          "type": ""
        },
        {
          "concept_constraint": "clanguml::t00056::iterable",
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T3",
          "type": ""
        },
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T4",
          "type": ""
        },
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T5",
          "type": ""
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00056::E<T1,T2,T3>",
      "id": "1429225801945621089",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": true,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "public",
          "is_static": false,
          "name": "e1",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 80
          },
          "type": "T1"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "e2",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 81
          },
          "type": "T2"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "e3",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 82
          },
          "type": "T3"
        }
      ],
      "methods": [],
      "name": "E",
      "namespace": "clanguml::t00056",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 79
      },
      "template_parameters": [
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T1",
          "type": ""
        },
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T2",
          "type": ""
        },
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T3",
          "type": ""
        }
      ],
      "type": "class"
    },
    {
      "bases": [],
      "display_name": "clanguml::t00056::F<T1,T2,T3>",
      "id": "856301122972546034",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": true,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "public",
          "is_static": false,
          "name": "f1",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 88
          },
          "type": "T1"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "f2",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 89
          },
          "type": "T2"
        },
        {
          "access": "public",
          "is_static": false,
          "name": "f3",
          "source_location": {
            "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
            "line": 90
          },
          "type": "T3"
        }
      ],
      "methods": [],
      "name": "F",
      "namespace": "clanguml::t00056",
      "source_location": {
        "file": "/home/bartek/devel/clang-uml/tests/t00056/t00056.cc",
        "line": 87
      },
      "template_parameters": [
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T1",
          "type": ""
        },
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T2",
          "type": ""
        },
        {
          "is_template_parameter": true,
          "is_template_template_parameter": false,
          "is_variadic": false,
          "name": "T3",
          "type": ""
        }
      ],
      "type": "class"
    }
  ],
  "relationships": [
    {
      "destination": "385255522691733325",
      "label": "T",
      "source": "137304962071054497",
      "type": "constraint"
    },
    {
      "destination": "392540961352249242",
      "label": "T",
      "source": "1043398062146751019",
      "type": "constraint"
    },
    {
      "destination": "1850394311226276678",
      "label": "T",
      "source": "1043398062146751019",
      "type": "constraint"
    },
    {
      "destination": "1043398062146751019",
      "label": "T",
      "source": "866345615551223718",
      "type": "constraint"
    },
    {
      "destination": "385255522691733325",
      "label": "T",
      "source": "866345615551223718",
      "type": "constraint"
    },
    {
      "destination": "385255522691733325",
      "label": "T",
      "source": "1418333499545421661",
      "type": "constraint"
    },
    {
      "destination": "866345615551223718",
      "label": "T",
      "source": "1814355496814977880",
      "type": "constraint"
    },
    {
      "destination": "137304962071054497",
      "label": "T",
      "source": "1512618198241549089",
      "type": "constraint"
    },
    {
      "destination": "392540961352249242",
      "label": "T1",
      "source": "1635109601630198093",
      "type": "constraint"
    },
    {
      "destination": "392540961352249242",
      "label": "T3",
      "source": "1635109601630198093",
      "type": "constraint"
    },
    {
      "destination": "385255522691733325",
      "label": "T2",
      "source": "1635109601630198093",
      "type": "constraint"
    },
    {
      "destination": "385255522691733325",
      "label": "T5",
      "source": "1635109601630198093",
      "type": "constraint"
    },
    {
      "destination": "1830716585637735576",
      "label": "T1,T3",
      "source": "1429225801945621089",
      "type": "constraint"
    },
    {
      "destination": "902541696362244204",
      "label": "T1,T3",
      "source": "856301122972546034",
      "type": "constraint"
    }
  ]
}
)##";
    auto j = generate_class_json(diagram, *model);

    REQUIRE(j == nlohmann::json::parse(expected_json));

    save_json(config.output_directory() + "/" + diagram->name + ".json", j);
}