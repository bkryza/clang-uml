# Configuration file reference

<!-- toc -->

* [Top level options](#top-level-options)
  * [Diagram options](#diagram-options)
* [Example complete config](#example-complete-config)
* [Determining config file location](#determining-config-file-location)
* [Paths](#paths)

<!-- tocstop -->

## Top level options
* `compilation_database_dir` - path to the directory containing `compile_commands.json`
* `output_directory` - path to the directory where PlantUML diagrams will be generated
* `diagrams` - the map of diagrams to be generated, each diagram name is provided as
             the key of the diagram YAML node
* `debug_mode` - add inline debug information in the generated diagrams
* `add_compile_flags` - add compile flags to all compilation database entries
* `remove_compile_flags` - remove compile flags from all compilation database entries
* `query_driver` - name or path to compiler driver, which should be queried for system include paths (e.g. arm-none-eabi-g++)

### Diagram options
* `type` - type of diagram, one of [`class`, `sequence`, `package`, `include`]
* `glob` - list of glob patterns to match source code files for analysis
* `include_relations_also_as_members` - when set to `false`, class members for relationships are rendered in UML are skipped from class definition (default: `true`)
* `generate_method_arguments` - determines whether the class diagrams methods contain full arguments (`full`), are abbreviated (`abbreviated`) or skipped (`none`)
* `generate_concept_requirements` - determines whether concept requirements are rendered in the diagram (default: `true`)
* `using_namespace` - similar to C++ `using namespace`, a `A::B` value here will render a class `A::B::C::MyClass` in the diagram as `C::MyClass`, at most 1 value is supported
* `generate_packages` - whether or not the class diagram should contain packages generated from namespaces or subdirectories
* `package_type` - determines how the packages are inferred: `namespace` - use C++ namespaces, `directory` - use project's directory structure
* `include` - definition of inclusion patterns:
    * `namespaces` - list of namespaces to include
    * `relationships` - list of relationships to include
    * `elements` - list of elements, i.e. specific classes, enums, templates to include
    * `element_types` - list of element types e.g. `enum`, `class`, `concept`
    * `access` - list of visibility scopes to include (e.g. `private`)
    * `subclasses` - include only subclasses of specified classes (and themselves)
    * `specializations` - include all specializations or instantiations of a given template
    * `dependants` - include all classes, which depend on the specified class
    * `dependencies` - include all classes, which are dependencies of the specified class
    * `context` - include only entities in direct relationship with specified classes
* `exclude` - definition of exclusion patterns:
    * `namespaces` - list of namespaces to exclude
    * `relationships` - list of relationships to exclude
    * `elements` - list of elements, i.e. specific classes, enums, templates to exclude
    * `element_types` - list of element types e.g. `enum`, `class`, `concept`
    * `access` - list of visibility scopes to exclude (e.g. `private`)
    * `subclasses` - exclude subclasses of specified classes (and themselves)
    * `specializations` - exclude all specializations or instantiations of a given template
    * `dependants` - exclude all classes, which depend on the specified class
    * `dependencies` - exclude all classes, which are dependencies of the specified class
    * `context` - exclude only entities in direct relationship with specified classes
* `layout` - add layout hints for entities (classes, packages)
* `plantuml` - verbatim PlantUML directives which should be added to a diagram
    * `before` - list of directives which will be added before the generated diagram
    * `after` - list of directives which will be added after the generated diagram
* `mermaid` - verbatim MermaidJS directives which should be added to a diagram
  * `before` - list of directives which will be added before the generated diagram
  * `after` - list of directives which will be added after the generated diagram
  * 
## Example complete config

```yaml
# Directory containing the compile_commands.json file
compilation_database_dir: debug
# Inject additional compile commands to the compilation database entries
add_compile_flags:
  - '-Wno-vla-extension'
# Remove specified compile flags from all compilation database entries
remove_compile_flags:
  - '-Wshadow'
# Compiler driver command to query for system include paths
query_driver:
  - arm-none-eabi-g++
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
    # Generate packages from the namespaces
    generate_packages: true
    package_type: namespace  # or 'directory' to generate from projects subdirectories
    # Limiting the number of files to include can significantly
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
      # Only include elements in direct relationship with ClassA
      context:
        - ClassA
    exclude:
      # Do not include private members and methods in the diagram
      access:
        - private
    layout:
      # Add layout hints for PlantUML
      ClassA:
        - up: ClassB
        - left: ClassC
      ClassD:
        - together: [ClassE, ClassF, EnumG]
      ClassX:
        - row: [ClassY1, ClassZ1]
        - column: [ClassY2, ClassZ2]
    # Specify customized relationship hints for types which are
    # arguments in template instantiations
    relationship_hints:
      # All tuple arguments should be treated as aggregation
      std::tuple: aggregation
      # All some_template arguments should be treated as associations
      # except for arguments with indexes 2 and 10
      ns1::n2::some_template:
        default: association
        2: composition
        10: aggregation          
    # Entities from this namespace will be shortened
    # (can only contain one element at the moment)
    using_namespace:
      - clanguml::class_diagram::model
    plantuml:
      # Add this line to the beginning of the resulting puml file
      before:
        - 'title clang-uml class diagram model'
    mermaid:
      # Add this line at the end of a Mermaid diagram
      end:
        - 'direction LR'
```

## Determining config file location

If `-c,--config` option is not provided, `clang-uml` will try to open file
`.clang-uml` in the current directory and fail if it doesn't exist.

With `-c,--config` option pointing to a valid `.clang-uml` file path, the file
configuration will be loaded from that file.

Furthermore, when the value of `-c,--config` option is `-` - the entire config
will be read from `stdin`, which can be useful for scripting and generating
config Yaml documents on the fly.

## Paths
By default, all paths specified in the configuration file, including:

* `glob`
* `output_directory`
* `compilation_database_dir`
* `paths` filter

are relative to the parent directory of the configuration file. This can be
changed in the following ways:
* by specifying `relative_to` option in the configuration file
* by providing `--paths-relative-to-pwd` command line option, in which
  case all paths will be relative to the directory where `clang-uml` is
  executed (this only makes sense for automation where `clang-uml` is executed
  from the same location relative to the project directory