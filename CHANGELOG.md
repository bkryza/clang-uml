# CHANGELOG

  * Added regexp support to selected diagram filters (#51, #132)
  * Added method type diagram filter (#145)
  * Added default method grouping and sorting in class diagrams (#36)
  * Improved generation of method attributes (e.g. constexpr, noexcept) (#142)

### 0.3.6
  * Added generation of packages in class and package diagrams from
    filesystem directories (#144)
  * Improved handling of class template specializations and their
    relationships (#140)
  * Fixed handling of C99 typedef structs (#138)

### 0.3.5
  * Added `--query-driver` option to automatically detect system include paths (#109)
  * Fixed add_compile_flags and added remove_compile_flags config options (#130)
  * Added rendering of template specialization fields and methods (#128)
  * Improved template specialization/instantiation matching based on deduced
    context

### 0.3.4
  * Added diagram metadata to PlantUML and JSON generators (#27)
  * Improved template specialization matching for variadic and function 
    template parameters (#118)
  * Fixed compilation and tests on LLVM 16 (#108)

### 0.3.3
  * Added 'add_compile_flags' config options (#112)
  * Added JSON generator (#114)
  * Added diagram templates support (#105)
  * Added parents (base classes) diagram filter
  * Fixed namespace handling for nested template specializations

### 0.3.2
  * Added initial support for C++20 concept rendering (#96)
  * Added support for plain C11 translation units (#97)
  * Added 'row' and 'column' layout hints for aligning elements (#90)
  * Added 'together' layout hint for grouping elements (#43)
  * Enabled adding notes to class methods and members (#87)
  * Improved rendering of template methods in class diagrams (#83)

### 0.3.1
  * Added relationship deduction from `auto` return types which are template
    specializations (#82)
  * Fixed lambda names in class diagrams to be based on relative paths
    with respect to `relative_to` config option (#78)
  * Fixed relative paths in config files to be relative to the parent of
    the configuration file by default (#69)
  * Added command line option (--dump-config) to print effective config (#77)
  * Added support for building with Microsoft Visual Studio

### 0.3.0
  * Added support for sequence diagrams with template code

### 0.2.2
  * Added structured comment parsing (#32)
  * Fixed namespace exclusive filtering

### 0.2.1
  * Fixed handling of classes nested in templates and anonymous nested structs
  * Fixed handling of configurable type aliases

### 0.2.0
  * Refactored translation units visitors from libclang to Clang LibTooling (#50)
  * Fixed root namespace handling (#45)
  * Removed `static` prefix from constructors

### 0.1.0
  * Initial release
