# Contributing to clang-uml

Thanks for taking interest in `clang-uml`!

> Please, make sure you're ok with 
> [Code of conduct](./CODE_OF_CONDUCT.md)
> and [LICENSE](./LICENSE.md)


## If you found a bug

* Optimally, fork the repository, create some branch, and add a test case reproducing the issue (it's easy!) - 
  check what is the highest test case number in a specific category and use the test case generation script with
  a consecutive number:
   ```bash
   # To generate a class diagram test case with number 50
   ./util/generate_test_case.py class 50
   ```
  This will generate a new test case template in the `tests/t00050` directory. In `tests/t00050/t00050.cc` write
  the C++ code which triggers the issue, and in `tests/t00050/test_case.h` write the test checks.
  The test case must be also added manually to `tests/test_cases.cc`:
   ```cpp
   // ...
   #include "t00047/test_case.h"
   #include "t00048/test_case.h"
   #include "t00049/test_case.h"
   #include "t00050/test_case.h" // <<<
   // ...
   ```
  
  Finally, create an issue with a link to your branch with the new test case.

* If the issue occurs with some publicly available code (e.g. available on GitHub), please create a new issue
  and describe the steps necessary to reproduce the issue including:
  * Link to source code and specific version used
  * `clang-uml` version used
  * Operating system, compiler and linked LLVM version
  * `.clang-uml` configuration file used
  * Exact command line used to execute `clang-uml`
  * Excerpt from the console logs 

* If the code on which `clang-uml` fails cannot be shared, please create an issue and try to provide at least:
    * `clang-uml` version used
    * Operating system, compiler and linked LLVM version
    * `.clang-uml` configuration file used
    * Exact command line used to execute `clang-uml`
    * Excerpt from the console logs 
    * Description of the code fragment on which the error occurs (e.g. a variadic template or anonymous struct)

## If you would like to fix a bug
* Fork the repository and create a branch
* Create a new test case which triggers the bug (see above how to create a new test case)
  ```bash
  # Make sure the test case fails
  make test
  ```
* Fix the code so that the test case passes
  ```bash
  # Again, make sure the test case fails and other still pass
  make test
  ```
* Commit the changes including all new files
* Make sure the code is properly formatted:
  ```bash
  # Requires docker to ensure consistent formatting through specific clang-format version
  make format
  git add . && git commit -m "Fixed formatting"
  ```
* Make sure the code doesn't introduce any `clang-tidy` warnings:
  ```bash
  make tidy
  ```

* Create a pull request from your branch to `master` branch

## If you would like to add a feature
* First, please check if the feature isn't already mentioned in the issues or existing PR's
* If not, create a new issue and describe as good as possible the new feature including:
  * Rationale
  * If the feature adds new configuration options, provide an example of new configuration file
  * If the feature adds a new diagram feature, please add an example C++ code and expected PlantUML diagram which should
    be generated through the feature
* To maximize the chances of accepting the new feature, wait for some discussion on the issue before implementing
  the feature to ensure we're on the same page as to its purpose and possible implementation
* Next, implement the feature, please try to adapt to the overall code style:
  * 80-character line width
  * snakes not camels
  * use `make format` before submitting PR to ensure consistent formatting (requires Docker)
  * use `make tidy` to check if your code doesn't introduce any `clang-tidy` warnings
* Add test case (or multiple test cases), which cover the new feature
* Finally, create a pull request!


