# Troubleshooting

<!-- toc -->

* [General issues](#general-issues)
  * [clang-uml crashes when generating a diagram](#clang-uml-crashes-when-generating-a-diagram)
  * [Diagram generation is very slow](#diagram-generation-is-very-slow)
  * [Diagram generated with PlantUML is cropped](#diagram-generated-with-plantuml-is-cropped)
  * [Clang produces several warnings during diagram generation](#clang-produces-several-warnings-during-diagram-generation)
  * [Errors with C++20 modules and LLVM 18](#errors-with-c20-modules-and-llvm-18)
  * [Cannot generate diagrams from header-only projects](#cannot-generate-diagrams-from-header-only-projects)
  * [YAML anchors and aliases are not fully supported](#yaml-anchors-and-aliases-are-not-fully-supported)
  * [Schema validation error is thrown, but the configuration file is correct](#schema-validation-error-is-thrown-but-the-configuration-file-is-correct)
  * ["fatal error: 'stddef.h' file not found"](#fatal-error-stddefh-file-not-found)
* [Class diagrams](#class-diagrams)
  * [How can I generate class diagram of my entire project](#how-can-i-generate-class-diagram-of-my-entire-project)
  * [Cannot generate classes for 'std' namespace](#cannot-generate-classes-for-std-namespace)
* [Sequence diagrams](#sequence-diagrams)
  * [Generated diagram is empty](#generated-diagram-is-empty)
  * [Generated diagram contains several empty control blocks or calls which should not be there](#generated-diagram-contains-several-empty-control-blocks-or-calls-which-should-not-be-there)

<!-- tocstop -->

## General issues

### clang-uml crashes when generating a diagram

If `clang-uml` crashes with a segmentation fault, it is possible to trace the
exact stack trace of the fault using the following steps:

First, build `clang-uml` from source in debug mode, i.e.:

```bash
make debug
```

Then run `clang-uml`, preferably with `-vvv` for verbose log output. If your
`.clang-uml` configuration file contains more than 1 diagram, specify only
the diagram causing the crash, to make it easier to trace the root cause of
the crash, e.g.:

```bash
debug/src/clang-uml -vvv -n my_diagram
```

After `clang-uml` crashes again, detailed backtrace (generated using
[backward-cpp](https://github.com/bombela/backward-cpp) library) should be
visible on the console.

If possible, [create an issue](https://github.com/bkryza/clang-uml/issues) and
paste the stack trace and few last logs from the console.

### Diagram generation is very slow

`clang-uml` uses
Clang's [RecursiveASTVisitor](https://clang.llvm.org/doxygen/classclang_1_1RecursiveASTVisitor.html),
to traverse the source code. By default, this visitor is invoked on every
translation unit (i.e. each entry in your `compile_commands.json`), including
all of their header dependencies recursively. This means, that for large code
bases with hundreds or thousands of translation units, traversing all of them
will be slow (think `clang-tidy` slow...).

Fortunately, in most practical cases it is not necessary to traverse the entire
source code for each diagram, as all the information necessary to generate
a single diagram usually can be found in just a few translation units, or even
a single one.

This is where the `glob` configuration parameter comes in. It can be used to
limit the number of translation units to visit for a given diagram, for
instance:

```yaml
diagrams:
  ClassAContextDiagram:
    type: class
    # ...
    glob:
      - src/classA.cpp
    # ...
  InterfaceHierarchyDiagram:
    type: class
    # ...
    glob:
      - src/interfaces/*.cpp
    # ...
```

This should improve the generation times for individual diagrams significantly.

Furthermore, diagrams are generated in parallel if possible, by default using
as many threads as virtual CPU's are available on the system, however it can
be adjusted also manually using `-t` command line option.

### Diagram generated with PlantUML is cropped

When generating diagrams with PlantUML without specifying an output file format,
the default is PNG.
Unfortunately PlantUML will not check if the diagram will fit in the default PNG
size, and often the diagram
will be incomplete in the picture. A better option is to specify SVG as output
format and then convert
to PNG, e.g.:

```bash
plantuml -tsvg mydiagram.puml
convert +antialias mydiagram.svg mydiagram.png
```

### Clang produces several warnings during diagram generation

During the generation of the diagram `clang` may report a lot of warnings, which
do not occur during the compilation with other compiler (e.g. GCC). This can be
fixed easily by using the `add_compile_flags` config option. For instance,
assuming that the warnings are as follows:

```
... warning: implicit conversion from 'int' to 'float' changes value from 2147483647 to 2147483648 [-Wimplicit-const-int-float-conversion]
... warning: declaration shadows a variable in namespace 'YAML' [-Wshadow]
```

simply add the following to your `.clang-uml` configuration file:

```
add_compile_flags:
  - -Wno-implicit-const-int-float-conversion
  - -Wno-shadow
```

Alternatively, the same can be passed through the `clang-uml` command line, e.g.

```bash
clang-uml --add-compile-flag -Wno-implicit-const-int-float-conversion \
          --add-compile-flag -Wno-shadow ...
```

Please note that if your `compile_commands.json` already contains - for instance
`-Wshadow` - then you also have to remove it, i.e.:

```yaml
add_compile_flags:
  - -Wno-implicit-const-int-float-conversion
  - -Wno-shadow
remove_compile_flags:
  - -Wshadow
```

### Errors with C++20 modules and LLVM 18

When running `clang-uml` on code using C++20 modules, the LLVM version used to
build the project must be compatible with the LLVM version linked to
`clang-uml`, otherwise you'll get error like this:
```
fatal error: malformed or corrupted AST file: 'malformed block record in AST file'
```
or like this:
```
error: PCH file uses an older PCH format that is no longer supported
```

In particular versions 17 and 18 of LLVM are not compatible in this regard.

### Cannot generate diagrams from header-only projects

Currently, in order to generate UML diagrams using `clang-uml` it is necessary
that at least one translation unit (e.g. one `cpp`) exists and it is included
in the generated `compile_commands.json` database.

However, even if your project is a header only library, first check if the
generated `compile_commands.json` contains any entries - if yes you should be
fine - just make sure the `glob` pattern in the configuration file matches
any of them. This is due to the fact that most header only projects still have
test cases, which are compiled and executed, and which include the headers.
These are perfectly fine to be used as translation units to generate the
diagrams.

In case, the code really does not contain any translation units, you will have
to create one, typically a basic `main.cpp` which includes the relevant headers
should be fine.
Also, it's possible to simply create a separate project, with a single
translation unit, which includes the relevant headers and create diagrams
from there.

In the future there might be a workaround for this in `clang-uml`.

### YAML anchors and aliases are not fully supported

`clang-uml` uses [yaml-cpp](https://github.com/jbeder/yaml-cpp) library, which
currently does not support
[merging YAML anchor dictionaries](https://github.com/jbeder/yaml-cpp/issues/41),
e.g. in the following configuration file the `main_sequence_diagram` will work,
but the `foo_sequence_diagram` will fail with parse error:

```yaml
compilation_database_dir: debug
output_directory: output

.sequence_diagram_anchor: &sequence_diagram_anchor
  type: sequence
  glob: [ ]
  start_from:
    - function: 'main(int,const char**)'

diagrams:
  main_sequence_diagram: *sequence_diagram_anchor # This will work
  foo_sequence_diagram:
    <<: *sequence_diagram_anchor # This will not work
    glob: [ src/foo.cc ]
    start_from:
      - function: 'foo(int,float)'
```

One option around this is to use some YAML preprocessor, such as
[yq](https://github.com/mikefarah/yq) on such file and passing
the configuration file to `clang-uml` using `stdin`, e.g.:

```bash
yq 'explode(.)' .clang-uml | clang-uml --config -
```

### Schema validation error is thrown, but the configuration file is correct
Current version of `clang-uml` performs automatic configuration file
schema validation, and exits if the configuration file is invalid.

In case there is a bug in the schema validation, the schema validation
step can be skipped by providing `--no-validate` command line option.

### "fatal error: 'stddef.h' file not found"

This error means that Clang cannot find some standard headers in include
paths specified in the `compile_commands.json`. This typically happens on macOS
and sometimes on Linux, when the code was compiled with different Clang version
than `clang-uml` itself.

One solution to this issue is to add the following line to your `CMakeLists.txt`
file:

```cmake
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
```

Another option is to provide an option (on command line or in configuration
file) called `query_driver` (inspired by the [clangd](https://clangd.llvm.org/)
language server - although much less flexible), which will invoke the
provider compiler command and query it for its default system paths, which then
will be added to each compile command in the database. This is especially useful
on macOS as well as for embedded toolchains, example usage:

```bash
clang-uml --query-driver arm-none-eabi-g++
```

Another option is to make sure that the Clang is installed on the system (even
if not used for building your
project), e.g.:

```bash
apt install clang
```

If this doesn't help to include paths can be customized using config options:

* `add_compile_flags` - which adds a list of compile flags such as include paths
  to each entry of the compilation database
* `remove_compile_flags` - which removes existing compile flags from each entry
  of the compilation database

For instance:

```yaml
add_compile_flags:
  - -I/opt/my_toolchain/include
remove_compile_flags:
  - -I/usr/include
```

These options can be also passed on the command line, for instance:

```bash
clang-uml --add-compile-flag -I/opt/my_toolchain/include \
          --remove-compile-flag -I/usr/include ...
```

Also see
[here](./md_docs_2common__options.html#resolving-include-path-and-compiler-flags-issues).

## Class diagrams

### How can I generate class diagram of my entire project

I want to generate a diagram containing all classes and relationships in my
project - I don't care how big it is going to be.

Of course this is possible, the best way to do this is to specify
that `clang-uml`
should only include elements defined in files contained in project sources,
e.g.:

```yaml
diagrams:
  all_classes:
    type: class
    include:
      paths: [ include, src ]
```

As the diagram will be huge for even medium-sized projects, it will likely not
be readable. However, this option can be useful for cases when we want to get
a complete JSON model of the codebase using the JSON generator:

```bash
clang-uml -g json -n all_classes --progress
```

### Cannot generate classes for 'std' namespace

Currently, system headers are skipped automatically by `clang-uml`, due to
too many errors they produce when generating diagrams, especially when trying
to process `GCC`'s or `MSVC`'s system headers by `Clang` - not yet sure why
that is the case.

Basically it's best to either include only specific namespaces through the
inclusion filters, e.g.:

```yaml
include:
  namespaces:
    - myns1::myns12
```

or explicitly exclude `std` namespace:

```yaml
exclude:
  namespaces:
    - std
```

Hopefully this will be eventually resolved.

## Sequence diagrams

### Generated diagram is empty

In order to generate sequence diagram the location constraints (`from`, `to`
or `from_to`) in configuration file must point to valid locations in the code
for the diagram (e.g. `function`), which must match exactly the function
or method signature in the `clang-uml` diagram model.
Look for error in the console output such as:

```bash
Failed to find participant mynamespace::foo(int) for start_from condition
```

which means that either you have a typo in the function signature in the
configuration file, or that the function
was not defined in the translation units you specified in the `glob` patterns
for this diagram.

Except for simplest methods and functions, it is unlikely to write by hand
the exact string representation of the function signature as seen by `clang-uml`.

To find the exact function signature run `clang-uml` as follows:

```bash
clang-uml -n my_sequence_diagram --print-from | grep foo
```

Command line flag `--print-from` will print on stdout all functions
and methods available in the diagram model which can be used as starting
points for a sequence diagram (similarly `--print-to` can be used to list
all valid functions to be used as call chain end constraints).

### Generated diagram contains several empty control blocks or calls which should not be there

Currently, the filtering of call expressions and purging empty control blocks (
e.g. loops or conditional statements),
within which no interesting calls were included in the diagram is not perfect.
In case the regular `namespaces` filter
is not enough, it is useful to add also a `paths` filter, which will only
include participants and call expressions
from files in a subdirectory of your project, e.g.:

```yaml
   include:
     namespaces:
       - myproject
     paths:
       - src
```
