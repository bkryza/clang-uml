# Common diagram generation options

<!-- toc -->

* [Overall configuration file structure](#overall-configuration-file-structure)
* [Diagram titles](#diagram-titles)
* [Translation unit glob patterns](#translation-unit-glob-patterns)
* [Custom directives](#custom-directives)
* [Adding debug information in the generated diagrams](#adding-debug-information-in-the-generated-diagrams)
* [Resolving include path and compiler flags issues](#resolving-include-path-and-compiler-flags-issues)
  * [Use '--query-driver' command line option](#use---query-driver-command-line-option)
  * [Manually add and remove compile flags from the compilation database](#manually-add-and-remove-compile-flags-from-the-compilation-database)
  * [Using 'CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES'](#using-cmake_cxx_implicit_include_directories)
  * [Nix wrapper](#nix-wrapper)

<!-- tocstop -->

## Overall configuration file structure
By default, `clang-uml` will look for file `.clang-uml` in the project's
directory and read all diagram definitions configuration from it. The file must
be specified in YAML and it's overall structure is as follows:

```yaml
# Common options for all diagrams
# ...
# Diagram definitions
diagrams:
  first_diagram_name:
    type: class|sequence|package|include
    # Diagram specific options
    # ...
  second_diagram_name:
    type: class|sequence|package|include
    # Diagram specific options
    # ...
  # More diagrams
  # ...
```

The top level common options are inherited by specific diagrams, if the option
is applicable to them and they themselves do not override this option.

For detailed reference of all configuration options see [here](./configuration_file.md).

Effective configuration, including default values can be printed out in YAML
format using the following option:

```bash
clang-uml --dump-config
```

## Diagram titles
Each type of diagram can have a `title` property, which will be generated in the
diagram using directives specific to a given diagram generator, for instance:

```yaml
diagrams:
  diagram1:
    type: class
    title: Some explanatory diagram title
```

## Translation unit glob patterns
One of the key options of the diagram configuration is the list of translation
units, which should be parsed to get all necessary information for a diagram. 

The syntax is simple and based on glob patterns, which can be added to the
configuration file as follows:

```yaml
   glob:
     - src/dir1/*.cc
     - src/dir3/*.cc
```

The glob patterns only need to match the translation units, which are also in
the `compile_commands.json` file, i.e. any files that match the glob patterns,
but are not in `compile_commands.json` will be ignored. In case the `glob`
pattern set does not match any translation units an error will be printed on
the standard output.

For small projects, the `glob` property can be omitted, which will result in
`clang-uml` parsing all translation units from `compile_commands.json` for
the diagram. However, for large projects, constraining the number of translation
units for each diagram to minimum necessary to discover all necessary diagram
elements will significantly decrease the diagram generation times.

## Custom directives
In case it's necessary to add some custom PlantUML or MermaidJS declarations
before or after the generated diagram content, it can be achieved using
the `plantuml` or `mermaid` configuration properties, for instance for PlantUML:

```yaml
    plantuml:
      before:
        - left to right direction
      after:
        - note left of {{ alias("ns1::ns2::MyClass") }} This is my class. 
```

or for MermaidJS:

```yaml
    mermaid:
      before:
        - direction LR
      after:
        - note for {{ alias("ns1::ns2::MyClass") }} "This is my class." 
```

These directives are useful for instance for adding notes to elements in the
diagrams or customizing diagram layout and style.

Please note that when referring to diagram elements in PlantUML or MermaidJS
directives, they must be added using Jinja templates `alias` command as in the
example above.

More options can be found in the official docs for each respective generator:
  * [PlantUML](https://plantuml.com/)
  * [MermaidJS](https://mermaid.js.org/intro/) 

## Adding debug information in the generated diagrams
Sometimes it is useful for debugging issues with the diagrams to have information
on the exact source location, from which given declaration or call expression was
derived. By adding option:

```yaml
debug_mode: true
```

the generated PlantUML diagram will contain comments before each line containing
the source location of the specific diagram element.

## Resolving include path and compiler flags issues
Due to the fact, that a project can be compiled with different compilers
and toolchains, the system paths and compilation flags detected by the Clang
version linked to your `clang-uml` installation might differ from the ones
actually used to compile your project.

> This is often an issue on macOS, when `clang-uml` uses Homebrew version of LLVM
> and a project was built using system Apple Clang.

Typically, this results in error messages on the console during diagram
generation, such as:

```
... fatal: 'stddef.h' file not found
```

or

```
... warning: implicit conversion from 'int' to 'float' changes value from 2147483647 to 2147483648 [-Wimplicit-const-int-float-conversion]
```

These errors can be overcome, by ensuring that the Clang parser has the correct
include paths to analyse your code base on the given platform. `clang-uml`
provides several mechanisms to resolve this issue:

### Use '--query-driver' command line option

> This option is not available on Windows.

Providing this option on the `clang-uml` command line will result in `clang-uml`
executing the specified compiler with the following command, e.g.:

```bash
/usr/bin/c++ -E -v -x c /dev/null 2>&1
```

and extracting from the output the target and system include paths, which are
then injected to each entry of the compilation database. For instance, on my
system, when generating diagrams for an embedded project and providing
`arm-none-eabi-gcc` as driver:

```bash
clang-uml --query-driver arm-none-eabi-gcc
```

the following options are appended to each command line after `argv[0]` of the
command:

```bash
--target=arm-none-eabi -isystem /usr/lib/gcc/arm-none-eabi/10.3.1/include -isystem /usr/lib/gcc/arm-none-eabi/10.3.1/include-fixed -isystem /usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/include
```

If you want to include the system headers reported by the compiler specified
already as first argument of each compile command in your
`compile_commands.json`, you can simply invoke `clang-uml` as:

```bash
clang-uml --query-driver .
```

however please make sure that the `compile_commands.json` contains a command,
which is safe to execute.

### Manually add and remove compile flags from the compilation database
If the system paths extracted from the compiler are not sufficient to resolve
include paths issues, it is possible to manually adjust the compilation
flags by providing `add_compile_flags` and `remove_compile_flags` in the
configuration file, or providing `--add-compile-flag` and `--remove-compile-flag`
on the `clang-uml` command line.

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

### Using 'CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES'
Yet another option, for CMake based projects, is to use the following CMake option:

```cmake
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
```

### Nix wrapper
On NixOS or when using `nix`, `clang-uml` uses a wrapper script,
which builds and exports `CPATH` and `CPLUS_INCLUDE_PATH`
environment variables before running `clang-uml`, which contain valid
system header Clang paths for the current Nix environment.

If you want to use an unwrapped version, the `clang-uml-unwrapped` binary
can be called the same way as `clang-uml`.
