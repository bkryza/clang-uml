@mainpage clang-uml

# Documentation

`clang-uml` is an automatic C++ to UML class, sequence, package and include diagram generator, driven by
YAML configuration files. The main idea behind the
project is to easily maintain up-to-date diagrams within a code-base or document
legacy code. The configuration file or files for `clang-uml` define the
types and contents of each generated diagram.
The diagrams can be generated in [PlantUML](https://plantuml.com),
[MermaidJS](https://mermaid.js.org/) and JSON formats.

Example sequence diagram generated using `clang-uml` from [this code](https://github.com/bkryza/clang-uml/blob/master/tests/t20029/t20029.cc):
![Sample sequence diagram](test_cases/t20029_sequence.svg)

`clang-uml` currently supports C++ up to version 17 with partial support for C++ 20.

To see what `clang-uml` can do, checkout the diagrams generated for unit
test cases [here](./test_cases.md) or examples in
[clang-uml-examples](https://github.com/bkryza/clang-uml-examples) repository.

These pages provide both user and developer documentation.

* [Quick start](./quick_start.md)
* [Installation](./installation.md)
* **Generating diagrams**
  * [Common options](./common_options.md)
  * [Generator types](./generator_types.md)
  * [Class diagrams](./class_diagrams.md)
  * [Sequence diagrams](./sequence_diagrams.md)
  * [Package diagrams](./package_diagrams.md)
  * [Include diagrams](./include_diagrams.md)
  * [Diagram templates](./diagram_templates.md)
  * [Comment decorators](./comment_decorators.md)
  * [Diagram filters](./diagram_filters.md)
  * [Using Jinja templates in diagram configs](./jinja_templates.md)
  * [Interactive SVG diagrams using links](./interactive_svg_diagrams.md)
* [Configuration file reference](./configuration_file.md)
* [Doxygen integration](./doxygen_integration.md)
* [Test cases documentation](./test_cases.md)
* [Troubleshooting](./troubleshooting.md)
* [Changelog](./changelog.md)
* [License](./license.md)
* **Development**
  * [Architecture](./architecture.md)
  * [Contributing](./contributing.md)
