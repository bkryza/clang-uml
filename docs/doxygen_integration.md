# Doxygen integration

`clang-uml` diagrams can be easily added to the Doxygen documentation using the image tag, however
[Doxygen](https://www.doxygen.nl/index.html) does not support the `clang-uml` specific commands.

`clang-uml` decorators can be omitted completely in Doxygen, by adding the
following lines to the Doxygen config file:

```
ALIASES                += uml=""
ALIASES                += uml{1}=""
ALIASES                += uml{2}=""
ALIASES                += uml{3}=""
```