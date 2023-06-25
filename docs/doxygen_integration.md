# Doxygen integration

<!-- toc -->



<!-- tocstop -->

`clang-uml` diagrams can be easily added to the Doxygen documentation using the
`image` tag, however [Doxygen](https://www.doxygen.nl/index.html) does not
support the `clang-uml` specific [commands](./comment_decorators.md), which 
will  appear in the documentation unprocessed.

The best solution to this is to tell Doxygen to ignore them, by adding the
following lines to the Doxygen config file:

```
ALIASES                += uml=""
ALIASES                += uml{1}=""
ALIASES                += uml{2}=""
ALIASES                += uml{3}=""
```

Furthermore, Doxygen adds images to HTML documentation as `<img src=""/>`,
which disables interactive links in SVG diagrams. One way to go around it
is to use a special command for these images, for instance:

```
ALIASES                += embed{1}="\htmlonly <embed src=\"\1\"/> \endhtmlonly"
```

and then use the following in the source code comments:

```cpp
/**
 * @brief Base class for all diagram models
 *
 * @embed{diagram_hierarchy_class.svg}
 */
class diagram {
public:
// ...
```

This directive in the configuration file will add the SVG diagrams using
`<embed>` HTML tag, and enable the links in the diagram on most browsers.

Finally, to have `clang-uml` generate links from diagram elements such as classes
or packages to Doxygen pages, it is only necessary to add the following
configuration file option:

```yaml
generate_links:
  link: "{% if existsIn(element, \"doxygen_link\") %}{{ element.doxygen_link }}{% endif %}"
  tooltip: "{% if existsIn(element, \"comment\") and existsIn(element.comment, \"brief\") %}{{ abbrv(trim(replace(element.comment.brief.0, \"\\n+\", \" \")), 256) }}{% else %}{{ element.name }}{% endif %}"
```

This option will tell `clang-uml` to generate a link to a local Doxygen
documentation page, provided that it is possible to generate it. Currently,
this only works for diagram elements, it will not work for instance
for individual methods.

