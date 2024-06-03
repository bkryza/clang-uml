# CHANGELOG

 * Fixed handling of relationships to nested enums (#280)
 * Improved handling of anonymous and multi-dimensions arrays in
   class diagrams (#278)
 * Fixed handling of enums in class diagram context filter (#275)
 * Added Nix clang wrapper for improved include paths handling (Thanks
   @pogobanane)
 * Refactored test cases (#272) 

### 0.5.2
 * Fixed generation of empty packages in class diagrams (#253)
 * Added option inline_lambda_messages to omit lambda expressions from sequence
   diagrams (#261)
 * Added support for CUDA calls in sequence diagrams (#263)
 * Improved handling of message call comments (#264)
 * Fixed handling of nested lambda expressions in sequence diagrams
 * Fixed type aliases handling in sequence diagram message names (#260)
 * Added support for call expressions tracking through lambdas in function
   arguments (#168)
 * Added Nix build files (Thanks @hatch01, @uku3lig, @thomaslepoix)
 * Fixed building with LLVM 18 (#251)

### 0.5.1
 * Fixed elements filter in sequence diagrams (#248)
 * Fixed progress indicators ranges with multiple commands per TU (#240)
 * Added default diagram generation error for empty diagrams (#246)
 * Added style option to plantuml config section (#238)
 * Added generate_concept_requirements config option (#237)
 * Refactored util pipe handling (#244)
 * Fixed handling of variadic template parameters in sequence diagrams (#241)
 * Fixed handling of query_driver option in config file (#243)

### 0.5.0
 * Fixed static linking against LLVM (#225)
 * Fixed handling of absolute paths in glob patterns (#233)
 * Enabled type_aliases config option for sequence diagrams (#224)
 * Refactored and unified JSON generators output (#223)
 * Added support for C++20 module based packages in class diagrams (#101)
 * Added support for class diagram filtering based on C++20 modules (#195)
 * Added support for C++20 coroutines in class diagrams (#221)
 * Fixed progress indicator characters on Windows (#218)

### 0.4.2
 * Fixed random typos and omissions in docs (#208)
 * Fixed handling of diagram hyperlinks with sources outside of project dir (#213)
 * Fixed test case t00014 on macos (#176)
 * Added automatic generation of diagram images using PlantUML and MermaidJS (#204) 
 * Added radius parameter to context filter (#201)

### 0.4.1
 * Enabled manual call expression injection through comments (#196)
 * Added support for generating sequence diagram notes from comments (#194)
 * Added Bash and Zsh autocomplete scripts (#193)
 * Updated `clang-uml` to work with LLVM 17 (#190)
 * Fixed handling of compilation databases with relative header paths (#189)
 * Excluded package diagram dependencies on parent and child packages (186)
 * Excluded package diagram relationships to rejected packages (#185)
 * Added 'title' diagram property (#184)
 * Make sure sequence diagram messages generated during static variable
   initialization are rendered only once (#183)

### 0.4.0
 * Added MermaidJS diagram generators (#27)

### 0.3.9
  * Added `from_to` and `to` location constraints to sequence diagrams (#154)
  * Fixed 'else if' statement generation in sequence diagrams (#81)
  * Implemented removal of redundant dependency relationships (#28)
  * Add option to disable generation of dependency relation to template
    arguments (#141)
  * Added configuration file validation (#57)

### 0.3.8
  * Added option to display block conditional statements in sequence diagrams (#162)
  * Added Doxygen documentation (#161)
  * Added diagram generation progress indicators options (#158)
  * Extended source_location with column and translation unit info

### 0.3.7
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
