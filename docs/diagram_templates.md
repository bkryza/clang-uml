# Diagram templates

<!-- toc -->

* [Diagram template syntax](#diagram-template-syntax)
* [Adding templates to the configuration file](#adding-templates-to-the-configuration-file)
* [Adding diagram to configuration from a template](#adding-diagram-to-configuration-from-a-template)
* [Builtin templates](#builtin-templates)

<!-- tocstop -->

When working with large codebases, it is often necessary to create diagram
configurations for a large number of classes or groups of classes.

Diagram templates feature makes this easier, by allowing to define (or use
one of the builtin) templates and generate using them any number of diagram
configurations. Diagram templates can be defined using Jinja like template
syntax (current renderer implementation is
[inja](https://github.com/pantor/inja)).

## Diagram template syntax
Diagram templates are defined as part of the configuration file using the
following YAML node:

```yaml
diagram_templates:
  # Name of the template
  parents_hierarchy_tmpl:
    # Description of the template
    description: Generate class parents inheritance diagram
    # Diagram type
    type: class
    # Template definition - it has to be valid YAML after it is rendered
    template: |
      {{ diagram_name }}:
        type: class
        {% if exists("glob") %}
        glob: [{{ glob }}]
        {% endif %}
        {% if exists("using_namespace") %}
        using_namespace: {{ using_namespace }}
        {% endif %}
        include:
          parents: [{{ class_name }}]
          namespaces: [{{ namespace_names }}]
        relationships:
          - inheritance
        exclude:
          access: [public, protected, private]
        plantuml:
          before:
            - left to right direction
```

## Adding templates to the configuration file
Diagram templates can be added directly to the `.clang-uml` configuration file,
under a `diagram_templates:` key. However, for practical reasons it is better
to keep diagram template definitions in a separate Yaml file, and reference
it in the configuration file using `include!` directive, e.g.:

```yaml
diagram_templates:
  include!: .clang-uml-templates
```

## Adding diagram to configuration from a template
To add a new diagram definition to the configuration file based on the template,
simply call the `clang-uml` using the following options:

```bash
clang-uml --add-diagram-from-template parents_hierarchy_tmpl \
  --template-var class_name=clanguml::config::include_diagram \
  --template-var namespace_names=clanguml \
  --template-var diagram_name=abcd \
  --template-var "glob=src/config/*.cc" \
  --template-var using_namespace=clanguml::config
```

To see what variables are necessary to provide for a template, the template
can be printed from the command line using the following command:

```bash
clang-uml --show-template parents_hierarchy_tmpl
```

## Builtin templates
`clang-uml` has some predefined templates, which can be used directly. If the
users configuration file defines another template with a name which already
exists as built-in template it will override the predefined templates.

Currently, the following templates are built-in:
* `parents_hierarchy_tmpl` - generate inheritance hierarchy diagram including
   parents of a specified class
* `subclass_hierarchy_tmpl` - generate inheritance hierarchy diagram including
 c children of a specified class
* `class_context_tmpl` - generate diagram including all classes in direct
   relationship (of any kind) with a specific class
