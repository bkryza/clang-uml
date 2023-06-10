# Diagram filters

<!-- toc -->

* [`namespaces` _[string or regex]_](#namespaces-_string-or-regex_)
* [`elements` _[string or regex]_](#elements-_string-or-regex_)
* [`element_types`](#element_types)
* [`paths` _[string or glob]_](#paths-_string-or-glob_)
* [`context` _[string or regex]_](#context-_string-or-regex_)
* [`relationships`](#relationships)
* [`subclasses` _[string or regex]_](#subclasses-_string-or-regex_)
* [`parents` _[string or regex]_](#parents-_string-or-regex_)
* [`specializations` _[string or regex]_](#specializations-_string-or-regex_)
* [`access`](#access)
* [`method_types`](#method_types)
* [`dependants` and `dependencies` _[string or regex]_](#dependants-and-dependencies-_string-or-regex_)

<!-- tocstop -->

Diagram filters are at the core of generating diagrams with `clang-uml`, as they allow to fine tune the scope
of each diagram, and thus provide you with a several small, but readable diagrams instead of a single huge diagram
that cannot be effectively browsed, printed or included in an online documentation of your project.

Filters can be specified separate for each diagram, and they can be added as either `include` or `exclude` filters,
depending on which is more appropriate for a given diagram.

For instance to include only C++ entities from a namespace `ns1::ns2` but not `ns1::ns2::detail` add the following
to your diagram configuration:

```yaml
  include:
    namespaces:
      - ns1::ns2
  exclude:
    namespaces:
      - ns1::ns2::detail
```

Some filters accept either specified exact values, some support regular
expressions while some except glob patterns.

For filters which accept regular expressions, the regular expression has to
be provided as a map `re: 'pattern'` due to the fact the pointer (`*`) otherwise
would have to be escaped in situations such as `mycontainer<char*>`, so for
instance to specify that the diagram should exclude all classes containing the
word `test` simply add the following filter:

```yaml
exclude:
  elements:
    - re: '.*test.*'
```

`paths` filter is currently the only filter which accepts `glob` like patterns.

The following table specifies the values allowed in each filter:

| Filter name       | Possible values                  | Example values                                                                                                                                            |
|-------------------|----------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| `namespaces`      | Qualified name or regex          | - `ns1::ns2::ClassA` <br/>- `re: '.\*detail.\*'`                                                                                                          |
| `elements`        | Qualified name or regex          | - `ns1::ns2::ClassA` <br/>- `re: '.\*detail.\*'`                                                                                                          |
| `paths`           | File or dir path or glob pattern | - `src/dir1`<br/>- `src/dir2/a.cpp`<br/>- `src/dir3/*.cpp`                                                                                                |
| `context`         | Qualified name or regex          | - `ns1::ns2::ClassA`<br/>- `re: 'ns1::ns2::ClassA.+'`                                                                                                     |
| `relationships`   | Type of relationship             | - `inheritance`<br/>- `composition`<br/>- `aggregation`<br/>- `ownership`<br/>- `association`<br/>- `instantiation`<br/>- `friendship`<br/>- `dependency` |
| `subclasses`      | Qualified name or regex          | - `ns1::ns2::ClassA`<br/>- `re: 'ns1::ns2::ClassA.+'`                                                                                                     |
| `parents`         | Qualified name or regex          | - `ns1::ns2::ClassA`<br/>- `re: 'ns1::ns2::ClassA.+'`                                                                                                     |
| `specializations` | Qualified name or regex          | - `ns1::ns2::ClassA`<br/>- `re: 'ns1::ns2::ClassA.+'`                                                                                                     |
| `access`          | Method or member access scope    | - `public`<br/>- `protected`<br/>- `private`                                                                                                              |
| `method_types`    | Type of class method             | - `constructor`<br/>- `destructor`<br/>- `assignment`<br/>- `operator`<br/>- `defaulted`<br/>- `deleted`<br/>- `static`                                   |
| `dependants`      | Qualified name or regex          | - `ns1::ns2::ClassA`<br/>- `re: 'ns1::ns2::ClassA.+'`                                                                                                     |
| `dependencies`    | Qualified name or regex          | - `ns1::ns2::ClassA`<br/>- `re: 'ns1::ns2::ClassA.+'`                                                                                                     |

The following filters are available.

## `namespaces` _[string or regex]_

Allows to include or exclude entities from specific namespaces.

## `elements` _[string or regex]_

Allows to directly include or exclude specific entities from the diagrams, for instance to exclude a specific class
from an included namespace:

```yaml
  include:
    namespaces:
      - ns1::ns2
  exclude:
    elements:
      - ns1::ns2::MyClass
```

## `element_types`

Allows to include or exclude elements of specific type from the diagram, for instance
to remove all enums from a diagram add the following:

```yaml
  exclude:
    element_types:
      - enum
```

## `paths` _[string or glob]_

This filter allows to include or exclude from the diagram elements declared
in specific files.

```yaml
diagrams:
  t00061_class:
    type: class
    relative_to: ../../tests/t00061
    glob: [t00061.cc]
    include:
      paths: [include/t00061_a.h]
    using_namespace:
      - clanguml::t00061
```

Currently, this filter does not allow any globbing or wildcards, however
paths to directories can be specified.

## `context` _[string or regex]_

This filter allows to limit the diagram elements only to classes which are in direct relationship (of any kind) with
the specified class:

```yaml
  include:
    context:
      - ns1::ns2::MyClass
```


## `relationships`

This filter allows to include or exclude specific types of relationships from the diagram, for instance to only
include inheritance and template specialization/instantiation relationships add the following to the diagram:

```yaml
  include:
    relationships:
      - inheritance
      - instantiation
```

The following relationships can be used in this filter:
  * inheritance
  * composition
  * aggregation
  * ownership
  * association
  * instantiation
  * friendship
  * dependency

## `subclasses` _[string or regex]_

This filter allows to include or exclude all subclasses of a given class in the diagram.

## `parents` _[string or regex]_

This filter allows to include or exclude all parents (base classes) of a given class in the diagram.

## `specializations` _[string or regex]_

This filter allows to include or exclude specializations and instantiations of a specific template from the diagram.

## `access`

This filter allows to include or exclude class methods and members based on their access scope, allowed values are:

  * `public`
  * `protected`
  * `private`

## `method_types`

This filter allows to include or exclude various method types from the class diagram, allowed values are:
  * constructor
  * destructor
  * assignment
  * operator
  * defaulted
  * deleted
  * static

This filter is independent of the `access` filter, which controls which methods
are included based on access scope (e.g. `public`).

## `dependants` and `dependencies` _[string or regex]_

These filters allow to specify that only dependants or dependencies of a given class should be included in the diagram.
This can be useful for analyzing what classes in your project depend on some other class, which could have impact for
instance on refactoring.

For instance the following code:
```cpp

namespace dependants {
struct A {
};

struct B {
    void b(A *a) { }
};

struct BB {
    void bb(A *a) { }
};

struct C {
    void c(B *b) { }
};

struct D {
    void d(C *c) { }
    void dd(BB *bb) { }
};

struct E {
    void e(D *d) { }
};

struct F {
};
} // namespace dependants

namespace dependencies {

struct G {
};

struct GG {
};

struct H {
    void h(G *g) { }
    void hh(GG *gg) { }
};

struct HH {
    void hh(G *g) { }
};

struct I {
    void i(H *h) { }
};

struct J {
    void i(I *i) { }
};
```

and the following filter:
```yaml
    include:
      dependants:
        - clanguml::t00043::dependants::A
      dependencies:
        - clanguml::t00043::dependencies::J
      relationships:
        - dependency
```

generates the following diagram:

![t00043_class](./test_cases/t00043_class.svg)
