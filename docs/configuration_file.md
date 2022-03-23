# Configuration file reference

## Top level options
* `compilation_database_dir` - path to the directory containing `compile_commands.json`
* `output_directory` - path to the directory where PlantUML diagrams will be generated
* `diagrams` - the map of diagrams to be generated, each diagram name is provided as
             the key of the diagram YAML node

### Diagram options
* `type` - type of diagram, one of [`class`, `sequence`, `package`]
* `glob` - list of glob patterns to match source code files for analysis
* `include_relations_also_as_members` - when set to `false`, class members for relationships are rendered in UML are skipped from class definition (default: `true`)
* `generate_method_arguments` - determines whether the class diagrams methods contain full arguments (`full`), are abbreviated (`abbreviated`) or skipped (`none`)
* `using_namespace` - similar to C++ `using namespace`, a `A::B` value here will render a class `A::B::C::MyClass` in the diagram as `C::MyClass`
* `include` - definition of inclusion patterns:
    * `namespaces` - list of namespaces to include
    * `relationships` - list of relationships to include
    * `entity_types` - list of entity types to include (e.g. `class`, `enum`)
    * `scopes` - list of visibility scopes to include (e.g. `private`)
* `exclude` - definition of excqlusion patterns:
    * `namespaces` - list of namespaces to exclude
    * `relationships` - list of relationships to exclude
    * `entity_types` - list of entity types to exclude (e.g. `class`, `enum`)
    * `scopes` - list of visibility scopes to exclude (e.g. `private`)
* `layout` - add layout hints for entities (classes, packages)
* `plantuml` - verbatim PlantUML directives which should be added to a diagram
    * `before` - list of directives which will be added before the generated diagram
    * `after` - list of directives which will be added after the generated diagram

### Template engine
`clang-uml` integrates [inja](https://github.com/pantor/inja) template engine, with several
additional functions which can be used in textual directives within the configuration files,
notes and to generate links and tooltips to diagrams.

The following, are the `clang-uml` additional template functions:
* `ltrim(string)` - left trims a string
* `rtrim(string)` - right trims a string
* `trim(string)` - trims a string
* `substr(string, offset, length)` - returns a substring of a string from offset of length
* `split(string)` - splits a string and returns a list of strings
* `replace(string, regex, replacement)` - returns a string with replace matches to regex with replacement string
* `abbrv(string, length)` - returns a string truncated to length including trailing ellipsis
* `alias(string)` - returns a PlantUML alias of an C++ entity represented by string name


## Example complete config

```yaml
# Directory containing the compile_commands.json file
compilation_database_dir: debug
# The directory where *.puml files will be generated
output_directory: docs/diagrams
# Set this as default for all diagrams
generate_method_arguments: none
# Enable generation of hyperlinks to diagram elements
generate_links:
  # Link pattern
  link: "https://github.com/bkryza/clang-uml/blob/{{ git.commit }}/{{ element.source.path }}#L{{ element.source.line }}"
  # Tooltip pattern
  tooltip: "{{ element.name }}"
# The map of diagrams - keys are also diagram file names
diagrams:
  main_package:
    # Include this diagram definition from a separate file
    include!: uml/main_package_diagram.yml
  config_class:
    type: class
    # Do not include rendered relations in the class box
    include_relations_also_as_members: false
    # Limiting the number of files to include can significantly
    # improve the generation time
    glob:
      - src/common/model/*.h
      - src/common/model/*.cc
      - src/class_diagram/model/*.h
      - src/class_diagram/model/*.cc
    include:
      # Only include entities from the following namespaces
      namespaces:
        - clanguml::common::model
        - clanguml::class_diagram::model
    exclude:
      # Do not include private members and methods in the diagram
      scopes:
        - private
    layout:
      # Add layout hints for PlantUML
      ClassA:
        - up: ClassB
        - left: ClassC
    # Entities from this namespace will be shortened
    # (can only contain one element at the moment)
    using_namespace:
      - clanguml::class_diagram::model
    plantuml:
      # Add this line to the beginning of the resulting puml file
      before:
        - 'title clang-uml class diagram model'
```