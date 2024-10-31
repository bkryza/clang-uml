# Diagram filters

<!-- toc -->

* [namespaces](#namespaces)
* [modules](#modules)
* [elements](#elements)
* [element_types](#element_types)
* [paths](#paths)
* [context](#context)
  * [context radius](#context-radius)
  * [context direction](#context-direction)
  * [context relationships](#context-relationships)
* [relationships](#relationships)
* [subclasses](#subclasses)
* [parents](#parents)
* [specializations](#specializations)
* [access](#access)
* [module_access](#module_access)
* [method_types](#method_types)
* [callee_types](#callee_types)
* [dependants and dependencies](#dependants-and-dependencies)
* [anyof](#anyof)
* [allof](#allof)

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
expressions while some accept glob patterns.

For filters which accept regular expressions, the regular expression has to
be provided as a map ```r: 'pattern'``` due to the fact the pointer (```*```) otherwise
would have to be escaped in situations such as ```mycontainer<char*>```, so for
instance to specify that the diagram should exclude all classes containing the
word ```test``` simply add the following filter:

```yaml
exclude:
  elements:
    - r: '.*test.*'
```

The following table specifies the values allowed in each filter:

| Filter name       | Possible values                   | Example values                                                                                                                                          |
|-------------------|-----------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------|
| `namespaces`      | Qualified name or regex           | ```ns1::ns2::ClassA```, ```r: '.*detail.*'```                                                                                                           |
| `modules`         | Qualified name or regex           | ```mod1.mod2:par1```, ```r: '.*impl.*'```                                                                                                               |
| `elements`        | Qualified name or regex           | ```ns1::ns2::ClassA```, ```r: '.*detail.*'```                                                                                                           |
| `element_types`   | Types of diagram elements         | ```class```, ```enum```, ```concept```                                                                                                                  |
| `paths`           | File or dir path                  | ```src/dir1```, ```src/dir2/a.cpp```, ```src/dir3/*.cpp```                                                                                              |
| `context`         | Qualified name or regex           | ```ns1::ns2::ClassA```, ```r: 'ns1::ns2::ClassA.+'```                                                                                                   |
| `relationships`   | Type of relationship              | ```inheritance```, ```composition```, ```aggregation```, ```ownership```, ```association```, ```instantiation```, ```friendship```, ```dependency```    |
| `subclasses`      | Qualified name or regex           | ```ns1::ns2::ClassA```, ```r: 'ns1::ns2::ClassA.+'```                                                                                                   |
| `parents`         | Qualified name or regex           | ```ns1::ns2::ClassA```, ```r: 'ns1::ns2::ClassA.+'```                                                                                                   |
| `specializations` | Qualified name or regex           | ```ns1::ns2::ClassA```, ```r: 'ns1::ns2::ClassA.+'```                                                                                                   |
| `access`          | Method or member access scope     | ```public```, ```protected```, ```private```                                                                                                            |
| `module_access`   | Module access scope               | ```public```, ```private```                                                                                                                             |
| `method_types`    | Type of class method              | ```constructor```, ```destructor```, ```assignment```, ```operator```, ```defaulted```, ```deleted```, ```static```                                     |
| `dependants`      | Qualified name or regex           | ```ns1::ns2::ClassA```, ```r: 'ns1::ns2::ClassA.+'```                                                                                                   |
| `dependencies`    | Qualified name or regex           | ```ns1::ns2::ClassA```, ```r: 'ns1::ns2::ClassA.+'```                                                                                                   |
| `callee_types`    | Callee types in sequence diagrams | ```constructor```, ```assignment```, ```operator```, ```defaulted```, ```static```, ```method```, ```function```, ```function_template```, ```lambda``` |
| `anyof`           | Match any of the nested filters   | Logical operator, which returns true if any of its nested filters match (requires `filter_mode: advanced` option)                                       |
| `allof`           | Match all of the nested filters   | Logical operator, which returns true if all of its nested filters match (requires `filter_mode: advanced` option)                                       |

The following filters are available:

## namespaces

Allows to include or exclude entities from specific namespaces.

```yaml
  include:
    namespaces:
      - ns1::ns2
  exclude:
    namespaces:
      - ns1::ns2::detail
```

## modules

Allows to include or exclude entities from specific C++20 module.

```yaml
  include:
    modules:
      - mod1.mod2
  exclude:
    modules:
      - r: ".*impl.*"
```

## elements

Allows to directly include or exclude specific entities from the diagrams, for instance to exclude a specific class
from an included namespace simply add the following exclude block:

```yaml
  include:
    namespaces:
      - ns1::ns2
  exclude:
    elements:
      - ns1::ns2::MyClass
```

`elements` filter allows also for more fine-grained control over the diagram
contents. Instead of a literal value in the filter like above, the
filter can also specify to what type of element the filter applies.
For instance the following filter:

```yaml
  include:
    namespaces:
      - ns1::ns2
  exclude:
    elements:
      - ns1::ns2::MyClass
      - type: method
        name: ns1::ns2::OtherClass::log
```

will, in addition to excluding `ns1::ns2::MyClass`, also exclude `ns1::ns2::OtherClass::log` method.

Another useful example, for instance to ignore all setters and getters in the
code base, the method elements filter can be specified with a regex e.g.:

```yaml
  include:
    namespaces:
      - ns1::ns2
  exclude:
    elements:
      - type: method
        name:
          r: '.*::(get|set).*'
```

The `type` in this filter can be one of the following:
`any`, `function`, `function_template`, `class`, `enum`, `method`, `member`,
`concept`, `package`, `objc_method`, `objc_member`, `objc_protocol`,
`objc_category`, `objc_interface`.

## element_types

Allows to include or exclude elements of specific type from the diagram, for instance
to remove all enums from a diagram add the following:

```yaml
  exclude:
    element_types:
      - enum
```

## paths

This filter allows to include or exclude from the diagram elements declared
in specific files.

```yaml
diagrams:
  t00061_class:
    type: class
    glob: 
      - t00061.cc
    include:
      paths:
        - include/t00061_a.h
    using_namespace:
      - clanguml::t00061
```

Currently, this filter does not allow any globbing or wildcards, however
paths to directories can be specified.

## context

This filter allows to limit the diagram elements only to classes which are in
direct relationship (of any kind) with the specified `class`, `enum` or
`concept`:

```yaml
  include:
    context:
      - ns1::ns2::MyClass
```

### context radius
By default, the filter will only include or exclude items in direct
relationship (radius 1). It is however possible to define the context filter
and provide a custom radius:

```yaml
  include:
    context:
      - match:
          radius: 3
          pattern: ns1::ns2::MyClass
      - match:
          radius: 2
          pattern: ns1::ns2::MyOtherClass
```

Please note that you can specify multiple context filters in a single diagram
with different radius. Radius set to 0 will match only the given element.

### context direction
Sometimes we may be interested only in relationships pointing outward from
a specific element, or inward. For instance we may want to see which elements
depend through any relationship on a specified element. For this purpose the
context filter has a `direction` option, which can have one of 3 values:
  * `any` (default)
  * `inward` - only relationships pointing toward the specified class are
               considered in calculating the context
  * `outward` -  only relationships pointing toward the specified class are
               considered in calculating the context

For example, the following filter:
```yaml
      context:
        - match:
            radius: 2
            pattern: clanguml::t00076::B
            direction: inward
```
will include all elements related to class `B` through relationships pointing
toward `B` within a radius of maximum 2 relationships (see test case
[t00076](./test_cases/t00076.md)).

### context relationships
Another way to customize the context of a specified element is to declare
which relationships should be considered when calculating the elements' context.

For example, the following filter:
```yaml
      context:
        - match:
            radius: 1
            pattern: clanguml::t00078::A
            relationships:
              - inheritance
              - aggregation
```
will only include elements related to class `A` through inheritance or
aggregation relationship (see test case [t00076](./test_cases/t00078.md)).

## relationships

This filter allows to include or exclude specific types of relationships from the diagram, for instance to only
include inheritance and template specialization/instantiation relationships add the following to the diagram:

```yaml
  include:
    relationships:
      - inheritance
      - instantiation
```

The following relationships can be used in this filter:
  * `inheritance`
  * `composition`
  * `aggregation`
  * `ownership`
  * `association`
  * `instantiation`
  * `friendship`
  * `dependency`

## subclasses

This filter allows to include or exclude all subclasses of a given class in the diagram.

## parents

This filter allows to include or exclude all parents (base classes) of a given class in the diagram.

## specializations

This filter allows to include or exclude specializations and instantiations of a specific template from the diagram.

## access

This filter allows to include or exclude class methods and members based on their access scope, allowed values are:

  * `public`
  * `protected`
  * `private`

## module_access

This filter allows to include or exclude diagram elements based on the module in which they are declared, allowed values are:

* `public`
* `private`

## method_types

This filter allows to include or exclude various method types from the class diagram, allowed values are:
  * `constructor`
  * `destructor`
  * `assignment`
  * `operator`
  * `defaulted`
  * `deleted`
  * `static`

This filter is independent of the `access` filter, which controls which methods
are included based on access scope (e.g. `public`).

## callee_types

This filter is specific for `sequence diagrams` and allows to control, which
types of callees should be included/excluded from the diagram. In a sequence diagram,
a `callee` is the receiver of a message, and this filter specifies which types
of receivers should match.

The following callee types are supported:
  * `constructor`
  * `assignment`
  * `operator`
  * `defaulted`
  * `static`
  * `method`
  * `function`
  * `function_template`
  * `lambda`
  * `cuda_kernel`
  * `cuda_device`

## dependants and dependencies

These filters allow to specify that only dependants or dependencies of a given
class should be included in the diagram. This can be useful for analyzing what
classes in your project depend on some other class, which could have impact for
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

results in the following diagram:

![t00043_class](./test_cases/t00043_class.svg)

## anyof

> Requires setting `filter_mode: advanced` in diagram config

This filter is in fact a logical operator, which allows to gain more control
over how the specific diagram filters are combined. Consider for instance a
case where you want to include all elements from a specific namespace, as well
as some other elements from another namespace. With basic filter this is not
possible, as the `namespaces` filter will only allow elements from the
namespaces lister. However when using `anyof` it's possible to specify them
as an alternative, i.e. any element matching any of the `anyof` subfilters
will be included. For example:

```yaml
    include:
      anyof:
        subclasses:
          - ns1::nsA::A1
        namespaces:
          - ns2::nsB
        context:
          - ns3::nsC::B3
```

will include all subclasses of `ns1::nsA::A1`, all elements in the `ns2::nsB`
namespace as well as all elements in the context of element `ns3::nsC::B3`.

For more examples of this checkout test cases
[t00082](./test_cases/t00082_class.md) and
[t00083](./test_cases/t00083_class.md).

## allof

> Requires setting `filter_mode: advanced` in diagram config

This filter logical operator is complementary to the `anyof`. It matches all
diagram elements, which match all of it's subfilters. For instance the following
filter:

```yaml
    include:
      allof:
        namespaces:
          - clanguml
          - std
        context:
          - match:
              radius: 2
              pattern: clanguml::A
```

which will include all elements that are in the context of element `clanguml::A`
not farther than 2 relationships away and also they belong to either `clanguml`
or `std` namespace.
