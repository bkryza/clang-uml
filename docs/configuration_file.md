# Configuration file reference

## Top level options
* `compilation_database_dir` - path to the directory containing `compile_commands.json`
* `output_directory` - path to the directory where PlantUML diagrams will be generated
* `diagrams` - the map of diagrams to be generated, each diagram name is provided as
             the key of the diagram YAML node

### Diagram options
* `type` - type of diagram, one of [`class`, `sequence`, `package`, `include`]
* `glob` - list of glob patterns to match source code files for analysis
* `include_relations_also_as_members` - when set to `false`, class members for relationships are rendered in UML are skipped from class definition (default: `true`)
* `generate_method_arguments` - determines whether the class diagrams methods contain full arguments (`full`), are abbreviated (`abbreviated`) or skipped (`none`)
* `using_namespace` - similar to C++ `using namespace`, a `A::B` value here will render a class `A::B::C::MyClass` in the diagram as `C::MyClass`, at most 1 value is supported
* `include` - definition of inclusion patterns:
    * `namespaces` - list of namespaces to include
    * `relationships` - list of relationships to include
    * `elements` - list of elements, i.e. specific classes, enums, templates to include
    * `access` - list of visibility scopes to include (e.g. `private`)
    * `subclasses` - include only subclasses of specified classes (and themselves)
    * `specializations` - include all specializations or instantiations of a given template
    * `dependants` - include all classes, which depend on the specified class
    * `dependencies` - include all classes, which are dependencies of the specified class
    * `context` - include only entities in direct relationship with specified classes
* `exclude` - definition of excqlusion patterns:
    * `namespaces` - list of namespaces to exclude
    * `relationships` - list of relationships to exclude
    * `elements` - list of elements, i.e. specific classes, enums, templates to exclude
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
* `element(string)` - returns the entire JSON context a given diagram element, including the following properties:
  * `name` - name of the element 
  * `type` - type of diagram element (e.g. `class`, `enum`, `package`)
  * `namespace` - fully qualified element namespace
  * `full_name` - fully qualified element name
  * `comment` [optional] - elements comment, if any
  * `alias` - internal diagram element alias (e.g. PlantUML alias)
* `alias(string)` - returns a PlantUML alias of an C++ entity represented by string name
* `comment(string)` - returns a comment of an C++ entity represented by string name

Templates allow complex postprocessing of the diagrams, for instance creation of customized PlantUML
notes in the diagrams from comments in the code. Below is an example of using the above commands to
generate notes in the PlantUML diagram from code comments (see also test case [t00050](./test_cases/t00050.md)):

```yaml
    plantuml:
      after:
        # Add a note left of the `A` class with the entire clas comment as content
        - >
           note left of {{ alias("A") }}
              {{ comment("clanguml::t00050::A").formatted }}
           end note
        # Same as above
        - >
          note right of {{ element("clanguml::t00050::A").alias }}
             {% set e=element("clanguml::t00050::A") %} {{ e.comment.formatted }}
          end note
        # Add a note left of class 'C' using trimmed text content from the class comment
        - >
          note left of {{ alias("C") }} #AABBCC
             {{ trim(comment("clanguml::t00050::C").text) }}
          end note
        # For each element in the diagram (class, template, enum):
        #  - Add a note with \brief comment if exists
        #  - Add a note with \todo for each element which has it
        #  - Add a note with template parameter descriptions based on \tparam comment
        - >
          {# Render brief comments and todos, if any were written for an element #}
          {% for e in diagram.elements %}
          {% if existsIn(e, "comment") and existsIn(e.comment, "brief") %}

          note top of {{ e.alias }} {% if e.type == "class" %} #22AA22 {% else %} #2222AA {% endif %}
             {% set c=e.comment %} {{ c.brief.0 }}
          end note

          {% endif %}
          {% if existsIn(e, "comment") and existsIn(e.comment, "todo") %}
          {% set c=e.comment %}
          {% for t in c.todo %}
          note top of {{ e.alias }} #882222
             **TODO**
             {{ t }}
          end note

          {% endfor %}

          {% endif %}
          {# Render template paramete if any #}
          {% if existsIn(e, "comment") and existsIn(e.comment, "tparam") %}
          {% set c=e.comment %}

          note top of {{ e.alias }} #AAAAFF
             **Template parameters**
             {% for tp in c.tparam %}
             //{{ tp.name }}// {{ trim(tp.description) }}
             {% endfor %}
          end note

          {% endif %}
          {% endfor %}
```
Currently there are 2 available comment parsers:
* `plain` - default
* `clang`
They can be selected using `comment_parser` config option.

#### `plain` comment parser
This parser provides only 2 options to the Jinja context:
* `comment.raw` - raw comment text, including comment markers such as `///` or `/**`
* `comment.formatted` - formatted entire comment

#### `clang` comment parser
This parser uses Clang comment parsing API to extract commands from the command:
* `comment.raw` - raw comment text, including comment markers such as `///` or `/**`
* `comment.formatted` - formatted entire comment
* `comment.<command>.<N>` - where command is the command used in the command e.g. `brief`, `todo`, etc. 
   and `N` is the index of the command in the array (each comment can have multiple instances of the 
   same command such as `\todo`)
* `comment.text` - entire text of the comment that is not attached to any command
* `comment.paragraph.<N>` - array of plain text paragraphs, for instance if you don't use `\brief`
  commands but often provide brief description as first sentence of the comment separated with a new line
  from the rest of the comment

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
```