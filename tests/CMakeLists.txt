cmake_minimum_required(VERSION 3.12)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(TEST_DISABLE_WARNINGS "-Wno-unused-parameter -Wno-unused-private-field -Wno-unused-variable -Wno-attributes")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBCLANG_CXXFLAGS} ${TEST_DISABLE_WARNINGS}")

file(GLOB_RECURSE TEST_CASE_SOURCES t*/*.cc)
file(GLOB_RECURSE TEST_CASE_CONFIGS t*/.clang-uml)
file(GLOB_RECURSE TEST_CONFIG_YMLS test_config_data/*.yml)

set(CLANG_UML_TEST_LIBRARIES
        ${LIBCLANG_LIBRARIES}
        ${YAML_CPP_LIBRARIES}
        ${LIBTOOLING_LIBS}
        clang-umllib
        cppast
        Threads::Threads)

set(CLANG_UML_TEST_UTIL_SRC test_util.cc ${TEST_UTIL_SOURCES})
set(CLANG_UML_TEST_UTIL_HEADER catch.h)

set(CLANG_UML_TEST_MODEL_SRC test_model.cc ${TEST_MODEL_SOURCES})
set(CLANG_UML_TEST_MODEL_HEADER catch.h)

set(CLANG_UML_TEST_CASES_SRC test_cases.cc ${TEST_CASE_SOURCES})
set(CLANG_UML_TEST_CASES_HEADER catch.h)

set(CLANG_UML_TEST_DECORATOR_PARSER_SRC test_decorator_parser.cc ${TEST_UTIL_SOURCES})
set(CLANG_UML_TEST_DECORATOR_PARSER_HEADER catch.h)

set(CLANG_UML_TEST_CONFIG_SRC test_config.cc ${TEST_UTIL_SOURCES})
set(CLANG_UML_TEST_CONFIG_HEADER catch.h)

set(CLANG_UML_TEST_FILTERS_SRC test_filters.cc ${TEST_FILTERS_SOURCES})
set(CLANG_UML_TEST_FILTERS_HEADER catch.h)

set(CLANG_UML_TEST_THREAD_POOL_EXECUTOR_SRC test_thread_pool_executor.cc)
set(CLANG_UML_TEST_THREAD_POOL_EXECUTOR_HEADER catch.h)

add_executable(test_util ${CLANG_UML_TEST_UTIL_SRC} ${CLANG_UML_TEST_UTIL_HEADER})
target_link_libraries(test_util PRIVATE ${CLANG_UML_TEST_LIBRARIES})

add_executable(test_model ${CLANG_UML_TEST_MODEL_SRC} ${CLANG_UML_TEST_MODEL_HEADER})
target_link_libraries(test_model PRIVATE ${CLANG_UML_TEST_LIBRARIES})

add_executable(test_decorator_parser ${CLANG_UML_TEST_DECORATOR_PARSER_SRC} ${CLANG_UML_TEST_DECORATOR_PARSER_HEADER})
target_link_libraries(test_decorator_parser PRIVATE ${CLANG_UML_TEST_LIBRARIES})

add_executable(test_config ${CLANG_UML_TEST_CONFIG_SRC} ${CLANG_UML_TEST_CONFIG_HEADER})
target_link_libraries(test_config PRIVATE ${CLANG_UML_TEST_LIBRARIES})

add_executable(test_filters ${CLANG_UML_TEST_FILTERS_SRC} ${CLANG_UML_TEST_FILTERS_HEADER})
target_link_libraries(test_filters PRIVATE ${CLANG_UML_TEST_LIBRARIES})

add_executable(test_thread_pool_executor ${CLANG_UML_TEST_THREAD_POOL_EXECUTOR_SRC} ${CLANG_UML_TEST_THREAD_POOL_EXECUTOR_HEADER})
target_link_libraries(test_thread_pool_executor PRIVATE ${CLANG_UML_TEST_LIBRARIES})

add_executable(test_cases ${CLANG_UML_TEST_CASES_SRC} ${CLANG_UML_TEST_CASES_HEADER})
target_link_libraries(test_cases PRIVATE ${CLANG_UML_TEST_LIBRARIES})

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

foreach(TEST_CONFIG_YML ${TEST_CONFIG_YMLS})
    file(RELATIVE_PATH
            TEST_CONFIG_YML_RELATIVE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${TEST_CONFIG_YML})
    message(${TEST_CONFIG_YML_RELATIVE})
    configure_file(
            ${TEST_CONFIG_YML_RELATIVE}
            ${TEST_CONFIG_YML_RELATIVE}
            COPYONLY)
endforeach()

add_test(NAME test_util COMMAND test_util)
add_test(NAME test_decorator_parser COMMAND test_decorator_parser)
add_test(NAME test_config COMMAND test_config)
add_test(NAME test_model COMMAND test_model)
add_test(NAME test_thread_pool_executor COMMAND test_thread_pool_executor)
add_test(NAME test_cases COMMAND test_cases)
add_test(NAME test_filters COMMAND test_filters)
