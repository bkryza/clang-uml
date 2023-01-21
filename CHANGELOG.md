# CHANGELOG

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