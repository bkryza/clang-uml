cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(CLANG_UML_TEST_CASES_SRC
    test_cases.cc
    t00001/t00001.cc
    t00002/t00002.cc
    t00003/t00003.cc
)
set(CLANG_UML_TEST_CASES_HEADER
    catch.h
)

add_executable(test_cases
    ${CLANG_UML_TEST_CASES_SRC}
    ${CLANG_UML_TEST_CASES_HEADER})

target_link_libraries(test_cases
    PRIVATE
    ${LIBCLANG_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
    spdlog::spdlog clang-umllib)

configure_file(t00001/.clanguml t00001/.clanguml COPYONLY)
configure_file(t00002/.clanguml t00002/.clanguml COPYONLY)
configure_file(t00003/.clanguml t00003/.clanguml COPYONLY)

add_test(NAME test_cases COMMAND test_cases)
