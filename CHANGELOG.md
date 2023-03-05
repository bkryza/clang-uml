# CHANGELOG

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