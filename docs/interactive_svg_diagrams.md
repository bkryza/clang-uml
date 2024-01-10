# Interactive SVG diagrams

<!-- toc -->



<!-- tocstop -->

`clang-uml` in combination with PlantUML and MermaidJS link generation in
diagrams allows to generate interactive diagrams, where clicking on any class,
method or call expression can direct the user directly to the source code or
some other diagram or document available online.

For instance to generate links to GitHub repository directly for most diagram
elements simply add this to your `.clang-uml` file:

```yaml
generate_links:
  link: 'https://github.com/myorg/myrepo/blob/{{ git.commit }}/{{ element.source.path }}#L{{ element.source.line }}'
  tooltip: '{% if "comment" in element %}{{ abbrv(trim(replace(element.comment.formatted, "\n+", " ")), 256) }}{% else %}{{ element.name }}{% endif %}'
```

You can open example diagram [here](https://raw.githubusercontent.com/bkryza/clang-uml/master/docs/test_cases/t00014_class.svg) to see how it works in action.

Full documentation on how to use the [inja](https://github.com/pantor/inja)
template engine in `clang-uml` configuration files can be found
[here](./jinja_templates.md).
