# Template engine

<!-- toc -->

* [Accessing comment content](#accessing-comment-content)
  * ['plain' comment parser](#plain-comment-parser)
  * ['clang' comment parser](#clang-comment-parser)

<!-- tocstop -->

`clang-uml` integrates [inja](https://github.com/pantor/inja) template engine, with several
additional functions which can be used in textual directives within the configuration files,
notes and to generate links and tooltips in diagrams.

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

### Accessing comment content
Text available in the code comment blocks can be accessed in the templates depending on the selected comment
parser.

Currently there are 2 available comment parsers:
* `plain` - default
* `clang` - Clang's comment parser

They can be selected using `comment_parser` config option.

#### 'plain' comment parser
This parser provides only 2 options to the Jinja context:
* `comment.raw` - raw comment text, including comment markers such as `///` or `/**`
* `comment.formatted` - formatted entire comment

#### 'clang' comment parser
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