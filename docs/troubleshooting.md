# Troubleshooting

<!-- toc -->

* [General issues](#general-issues)
  * [Diagram generated with PlantUML is cropped](#diagram-generated-with-plantuml-is-cropped)
* [Class diagrams](#class-diagrams)
  * ["fatal error: 'stddef.h' file not found"](#fatal-error-stddefh-file-not-found)
* [Sequence diagrams](#sequence-diagrams)
  * [Generated diagram is empty](#generated-diagram-is-empty)
  * [Generated diagram contains several empty control blocks or calls which should not be there](#generated-diagram-contains-several-empty-control-blocks-or-calls-which-should-not-be-there)

<!-- tocstop -->

## General issues
### Diagram generated with PlantUML is cropped
When generating diagrams with PlantUML without specifying an output file format, the default is PNG. 
Unfortunately PlantUML will not check if the diagram will fit in the default PNG size, and often the diagram
will be be incomplete in the picture. A better option is to specify SVG as output format and then convert 
to PNG, e.g.:
```bash
$ plantuml -tsvg mydiagram.puml
$ convert +antialias mydiagram.svg mydiagram.png
```

## Class diagrams
### "fatal error: 'stddef.h' file not found"
This error means that Clang cannot find some standard headers in the include paths
specified in the `compile_commands.json`. This typically happens on macos and sometimes on Linux, when 
the code was compiled with different Clang version than `clang-uml` itself.

One solution to this issue is to add the following line to your `CMakeLists.txt` file:

```cmake
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
```

Another option is to make sure that the Clang is installed on the system (even if not used for building your
project), e.g.:
```bash
apt install clang
```

## Sequence diagrams
### Generated diagram is empty
In order to generate sequence diagram the `start_from` configuration option must have a valid starting point
for the diagram (e.g. `function`), which must match exactly the function signature in the `clang-uml` model.
Look for error in the console output such as:
```bash
Failed to find participant mynamespace::foo(int) for start_from condition
```
which means that either you have a typo in the function signature in the configuration file, or that the function
was not defined in the translation units you specified in the `glob` patterns for this diagram. Run again the
`clang-uml` tool with `-vvv` option and look in the console output for any mentions of the function from
which the diagram should start and copy the exact signature into the configuration file.

### Generated diagram contains several empty control blocks or calls which should not be there
Currently the filtering of call expressions and purging empty control blocks (e.g. loops or conditional statements),
within which no interesting calls were included in the diagram is not perfect. In case the regular `namespaces` filter
is not enough, it is useful to add also a `paths` filter, which will only include participants and call expressions
from files in a subdirectory of your project, e.g.:
```yaml
   include:
      namespaces:
         - myproject
      paths:
         - src
```