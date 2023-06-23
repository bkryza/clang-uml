# Generator types

<!-- toc -->

* [PlantUML](#plantuml)
* [JSON](#json)

<!-- tocstop -->

Currently, there are 2 types of diagram generators: `plantuml` and `json`.

To specify, which generators should be used on the command line use option `-g`.
For instance to generate both types of diagrams run `clang-uml` as follows:

```bash
clang-uml -g plantuml -g json
```

By default, only `plantuml` diagram are generated.

## PlantUML

Generates UML diagrams in textual PlantUML format, which can then
be converted to various image formats.

In case there is a need for adding custom PlantUML directives to generated
diagrams, they can be included directly in the diagram configuration. For
example:

```yaml
  plantuml:
    before:
      - title clang-uml configuration model
      - left to right direction
    after:
      - 'note left of {{ alias("inheritable_diagram_options") }}: Options common to all diagram types.'
      - 'note right of {{ alias("config") }}: General options not used by diagrams.'
```

will add before the diagram contents (right after `@startuml`) the title and
direction hint, and after each diagram contents (right before `@enduml`)
2 notes attached to elements.

An example PlantUML diagram is presented below:

```plantuml
@startuml
class "A" as C_0001371951663534295727
class C_0001371951663534295727 [[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L7{A}]] {
+A() = default : void
+A(int i) : void
+A(A &&) = default : void
+A(const A &) = deleted : void
+A<T>(T t) : void
+~A() = default : void
..
+operator=(A && other) noexcept : A &
+operator=(A & other) noexcept : A &
..
+operator++() : A &
..
+auto_method() : int
+basic_method() : void
+const_method() const : void
{static} +create_from_int(int i) : A
+default_int(int i = 12) : int
+default_string(int i, std::string s = "abc") : std::string
+double_int(const int i) : int
-private_method() : void
#protected_method() : void
+size() constexpr const : std::size_t
{static} +static_method() : int
+sum(const double a, const double b) : int
__
-a_ : int [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L70{a_}]]]
{static} +auto_member : const unsigned long [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L55{auto_member}]]]
-b_ : int [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L70{b_}]]]
-c_ : int [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L70{c_}]]]
#compare : std::function<bool (const int)> [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L62{compare}]]]
-private_member : int [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L69{private_member}]]]
#protected_member : int [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L60{protected_member}]]]
+public_member : int [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L52{public_member}]]]
{static} +static_const_int : const int [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L54{static_const_int}]]]
{static} +static_int : int [[[https://github.com/bkryza/clang-uml/blob/a39af67987036732468c95087191562780a518de/tests/t00003/t00003.cc#L53{static_int}]]]
}

'Generated with clang-uml, version 0.3.7-20-ga39af67
'LLVM version Ubuntu clang version 15.0.6
@enduml
```

The footer at the end is added by default, but can be disabled with
`--no-metadata` command line option.

## JSON

Generates a JSON representation of the intermediate `clang-uml` model, which
can be used for scripting, integrations as well as analysing the code base
or even generating diagrams in other formats.

An equivalent of the above PlantUML diagram in JSON is presented below:

```json
{
  "diagram_type": "class",
  "elements": [
    {
      "bases": [],
      "display_name": "clanguml::t00003::A",
      "id": "1371951663534295727",
      "is_abstract": false,
      "is_nested": false,
      "is_struct": false,
      "is_template": false,
      "is_union": false,
      "members": [
        {
          "access": "public",
          "is_static": false,
          "name": "public_member",
          "source_location": {
            "column": 9,
            "file": "../../tests/t00003/t00003.cc",
            "line": 52,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "int"
        },
        {
          "access": "protected",
          "is_static": false,
          "name": "protected_member",
          "source_location": {
            "column": 9,
            "file": "../../tests/t00003/t00003.cc",
            "line": 60,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "int"
        },
        {
          "access": "protected",
          "is_static": false,
          "name": "compare",
          "source_location": {
            "column": 36,
            "file": "../../tests/t00003/t00003.cc",
            "line": 62,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "std::function<bool (const int)>"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "private_member",
          "source_location": {
            "column": 9,
            "file": "../../tests/t00003/t00003.cc",
            "line": 69,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "int"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "a_",
          "source_location": {
            "column": 9,
            "file": "../../tests/t00003/t00003.cc",
            "line": 70,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "int"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "b_",
          "source_location": {
            "column": 13,
            "file": "../../tests/t00003/t00003.cc",
            "line": 70,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "int"
        },
        {
          "access": "private",
          "is_static": false,
          "name": "c_",
          "source_location": {
            "column": 17,
            "file": "../../tests/t00003/t00003.cc",
            "line": 70,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "int"
        },
        {
          "access": "public",
          "is_static": true,
          "name": "static_int",
          "source_location": {
            "column": 16,
            "file": "../../tests/t00003/t00003.cc",
            "line": 53,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "int"
        },
        {
          "access": "public",
          "is_static": true,
          "name": "static_const_int",
          "source_location": {
            "column": 22,
            "file": "../../tests/t00003/t00003.cc",
            "line": 54,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "const int"
        },
        {
          "access": "public",
          "is_static": true,
          "name": "auto_member",
          "source_location": {
            "column": 23,
            "file": "../../tests/t00003/t00003.cc",
            "line": 55,
            "translation_unit": "../../tests/t00003/t00003.cc"
          },
          "type": "const unsigned long"
        }
      ],
      "methods": [
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": true,
          "is_copy_assignment": false,
          "is_defaulted": true,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "A",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": true,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "A",
          "parameters": [
            {
              "name": "i",
              "type": "int"
            }
          ],
          "type": "void"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": true,
          "is_copy_assignment": false,
          "is_defaulted": true,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "A",
          "parameters": [
            {
              "name": "",
              "type": "clanguml::t00003::A &&"
            }
          ],
          "type": "void"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": true,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": true,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "A",
          "parameters": [
            {
              "name": "",
              "type": "const clanguml::t00003::A &"
            }
          ],
          "type": "void"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": true,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": true,
          "name": "~A",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "basic_method",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": true,
          "is_virtual": false,
          "name": "static_method",
          "parameters": [],
          "type": "int"
        },
        {
          "access": "public",
          "is_const": true,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "const_method",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "auto_method",
          "parameters": [],
          "type": "int"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": true,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "operator++",
          "parameters": [],
          "type": "clanguml::t00003::A &"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": true,
          "is_noexcept": true,
          "is_operator": true,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "operator=",
          "parameters": [
            {
              "name": "other",
              "type": "clanguml::t00003::A &&"
            }
          ],
          "type": "clanguml::t00003::A &"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": true,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": true,
          "is_operator": true,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "operator=",
          "parameters": [
            {
              "name": "other",
              "type": "clanguml::t00003::A &"
            }
          ],
          "type": "clanguml::t00003::A &"
        },
        {
          "access": "public",
          "is_const": true,
          "is_consteval": false,
          "is_constexpr": true,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "size",
          "parameters": [],
          "type": "std::size_t"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "double_int",
          "parameters": [
            {
              "name": "i",
              "type": "const int"
            }
          ],
          "type": "int"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "sum",
          "parameters": [
            {
              "name": "a",
              "type": "const double"
            },
            {
              "name": "b",
              "type": "const double"
            }
          ],
          "type": "int"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "default_int",
          "parameters": [
            {
              "default_value": "12",
              "name": "i",
              "type": "int"
            }
          ],
          "type": "int"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "default_string",
          "parameters": [
            {
              "name": "i",
              "type": "int"
            },
            {
              "default_value": "\"abc\"",
              "name": "s",
              "type": "std::string"
            }
          ],
          "type": "std::string"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": true,
          "is_virtual": false,
          "name": "create_from_int",
          "parameters": [
            {
              "name": "i",
              "type": "int"
            }
          ],
          "type": "clanguml::t00003::A"
        },
        {
          "access": "protected",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "protected_method",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "private",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": false,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "private_method",
          "parameters": [],
          "type": "void"
        },
        {
          "access": "public",
          "is_const": false,
          "is_consteval": false,
          "is_constexpr": false,
          "is_constructor": true,
          "is_copy_assignment": false,
          "is_defaulted": false,
          "is_deleted": false,
          "is_move_assignment": false,
          "is_noexcept": false,
          "is_operator": false,
          "is_pure_virtual": false,
          "is_static": false,
          "is_virtual": false,
          "name": "A",
          "parameters": [
            {
              "name": "t",
              "type": "T"
            }
          ],
          "type": "void"
        }
      ],
      "name": "A",
      "namespace": "clanguml::t00003",
      "source_location": {
        "column": 7,
        "file": "../../tests/t00003/t00003.cc",
        "line": 7,
        "translation_unit": "../../tests/t00003/t00003.cc"
      },
      "template_parameters": [],
      "type": "class"
    }
  ],
  "metadata": {
    "clang_uml_version": "0.3.7-20-ga39af67",
    "llvm_version": "Ubuntu clang version 15.0.6",
    "schema_version": 1
  },
  "name": "t00003_class",
  "relationships": [],
  "using_namespace": "clanguml::t00003"
}
```

