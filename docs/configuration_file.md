# Configuration file reference

## Top level options
* `compilation_database_dir` - path to the directory containing `compile_commands.json`
* `output_directory` - path to the directory where PlantUML diagrams will be generated
* `diagrams` - the map of diagrams to be generated, each diagram name is provided as
             the key of the diagram YAML node

### Diagram options
* `type` - type of diagram, one of [`class`, `sequence`]
* `glob` - list of glob patterns to match source code files for analysis
* `using_namespace` - similar to C++ `using namespace`, a `A::B` value here will render a class `A::B::C::MyClass` in the diagram as `C::MyClass`
* `include` - definition of inclusion patterns:
    * `namespaces` - list of namespaces to include
    * `relationships` - list of relationships to include
    * `entity_types` - list of entity types to include (e.g. `class`, `enum`)
    * `scopes` - list of visibility scopes to include (e.g. `private`)
* `exclude` - definition of exclusion patterns:
    * `namespaces` - list of namespaces to exclude
    * `relationships` - list of relationships to exclude
    * `entity_types` - list of entity types to exclude (e.g. `class`, `enum`)
    * `scopes` - list of visibility scopes to exclude (e.g. `private`)
* `plantuml` - verbatim PlantUML directives which should be added to a diagram
    * `before` - list of directives which will be added before the generated diagram
    * `after` - list of directives which will be added after the generated diagram
