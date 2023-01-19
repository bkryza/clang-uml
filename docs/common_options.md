# Common diagram generation options

<!-- toc -->

* [Overall configuration file structure](#overall-configuration-file-structure)
* [Translation unit glob patterns](#translation-unit-glob-patterns)
* [PlantUML custom directives](#plantuml-custom-directives)
* [Adding debug information in the generated diagrams](#adding-debug-information-in-the-generated-diagrams)

<!-- tocstop -->

## Overall configuration file structure
By default, `clang-uml` will look for file `.clang-uml` in the projects directory and read all diagrams definitions
from it. The file must be specified in YAML and it's overall structure is as follows:

```yaml
<common options for all diagrams>
diagrams:
  <first diagram name>:
    type: [class|sequence|package|include]
    <diagram specific options>
  <second diagram name>:
    type: [class|sequence|package|include]
    <diagram specific options>
  ...
```

The top level common options are inherited by specific diagrams, if the option is applicable to them and they themselves
do not override this option.

For detailed reference of all configuration options see [here](./configuration_file.md).

Effective configuration, including default values can be printed out in YAML format using the following option:

```bash
clang-uml --dump-config
```

## Translation unit glob patterns
One of the key options of the diagram configuration is the list of translation units, which should be parsed to
get all necessary information for a diagram. 

The syntax is simple and based on glob patterns, which can be added to the configuration file as follows:

```yaml
   glob:
     - src/dir1/*.cc
     - src/dir3/*.cc
```

The glob patterns only need to match the translation units, which are also in the `compile_commands.json` file, i.e.
any files that match the glob patterns but are not in `compile_commands.json` will be ignored. In case the `glob`
pattern set does not much any translation units an error will be printed on the standard output.

For small projects, the `glob` property can be omitted, which will result in `clang-uml` parsing all translation units
from `compile_commands.json` for the diagram. However for large projects, constraining the number of translation units
for each diagram to absolute minimum will significantly decrease the diagram generation times.

## PlantUML custom directives
In case it's necessary to add some custom PlantUML declarations before or after the generated diagram content,
it can be achieved simply using the `plantuml` configuration properties, for instance:

```yaml
    plantuml:
      before:
        - left to right direction
      after:
        - note left of {{ alias("ns1::ns2::MyClass") }} This is my class. 
```

These directive are useful for instance for adding notes to elements in the diagrams or customizing diagram layout
or style.

Please note that when referring to diagram elements in the PlantUML directives, they must be added using Jinja 
templates `alias` command as in the example above.

More options can be found in the official PlantUML [documentation](https://plantuml.com/).

## Adding debug information in the generated diagrams
Sometimes it is useful for debugging issues with the diagrams to have information on the exact source location,
from which given declaration or call expression was derived. By adding option:

```yaml
debug_mode: true
```

the generated PlantUML diagram will contain comments before each line containing the source location of the
specific diagram element.