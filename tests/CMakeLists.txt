cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE TEST_CASE_SOURCES t*/*.cc)
file(GLOB_RECURSE TEST_CASE_CONFIGS t*/.clanguml)

set(CLANG_UML_TEST_UTIL_SRC
    test_util.cc
    ${TEST_UTIL_SOURCES}
)
set(CLANG_UML_TEST_UTIL_HEADER
    catch.h
)

set(CLANG_UML_TEST_CASES_SRC
    test_cases.cc
    ${TEST_CASE_SOURCES}
)
set(CLANG_UML_TEST_CASES_HEADER
    catch.h
)

add_executable(test_util
    ${CLANG_UML_TEST_UTIL_SRC}
    ${CLANG_UML_TEST_UTIL_HEADER})

target_link_libraries(test_util
    PRIVATE
    ${LIBCLANG_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
    spdlog::spdlog clang-umllib cppast)


add_executable(test_cases
    ${CLANG_UML_TEST_CASES_SRC}
    ${CLANG_UML_TEST_CASES_HEADER})

target_link_libraries(test_cases
    PRIVATE
    ${LIBCLANG_LIBRARIES}
    ${YAML_CPP_LIBRARIES}
    spdlog::spdlog clang-umllib cppast)

foreach(TEST_CASE_CONFIG ${TEST_CASE_CONFIGS})
    file(RELATIVE_PATH
        TEST_CASE_CONFIG_RELATIVE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${TEST_CASE_CONFIG})
    message(${TEST_CASE_CONFIG_RELATIVE})
    configure_file(
        ${TEST_CASE_CONFIG_RELATIVE}
        ${TEST_CASE_CONFIG_RELATIVE}
        COPYONLY)
endforeach()

add_test(NAME test_util COMMAND test_util)
add_test(NAME test_cases COMMAND test_cases)
