cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

# Explicitly list the test source code and headers. The Catch header-only unit
# test framework is stored in with the test source.
set(CLANG_UML_TEST_EXAMPLE_SRC
    test_example.cpp
)
set(CLANG_UML_TEST_EXAMPLE_HEADER
    catch.hpp
)

# Make an executable target that depends on the test source code we specified
# above.
add_executable(test-example ${CLANG_UML_TEST_EXAMPLE_SRC} ${CLANG_UML_TEST_EXAMPL_HEADER})

# Enable testing via CTest
enable_testing()

# Add our test as runnable via CTest
add_test(NAME test-example COMMAND test-example)
