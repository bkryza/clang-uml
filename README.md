# clang-uml - UML diagram generator based on Clang and PlantUML

clang-uml is an automatic PlantUML class and sequence diagram generator, driven by
YAML configuration files. The main idea behind the project is to easily
maintain up-to-date diagrams within a code-base.  The configuration file or
files for clang-uml define the type and scope of each diagram.

## Rationale

## Installation

TODO

## Usage

### Invocation

### Configuration file format and examples

### Class diagrams

#### Default mappings

| UML                          | C++                                                                                                |
| ----                         | ---                                                                                                |
| Inheritance (A is kind of B) | Public, protected or private inheritance                                                           |
| Association (A knows of B)   | Class A has a pointer or a reference to class B, or any container with a pointer or reference to B |
| Dependency (A uses B)        | Any method of class A has argument of type B                                                       |
| Aggregation (A has B)        | Class A has a field of type B or an owning pointer of type B                                       |
| Composition (A has B)        | Class A has a field of type container of B                                                         |
| Template (T specializes A)   | Class A has a template parameter T                                                                 |
| Nesting (A has inner class B)| Class B is an inner class of A

#### Inline directives

## Building

### Ubuntu
To build under Ubuntu follow the following steps:
```bash
    sudo apt-get install cmake build-essential libyaml-cpp-dev libspdlog-dev libclang-11-dev libclang-cpp11-dev
    git clone https://github.com/bkryza/clang-uml
    cd clang-uml
    make release # or make debug for debug builds
```

## LICENSE

    Copyright 2021-present Bartek Kryza <bkryza@gmail.com>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

