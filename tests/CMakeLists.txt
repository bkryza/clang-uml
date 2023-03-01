
file(GLOB_RECURSE TEST_CASE_SOURCES t*/*.cc t*/*.c t*/src/*.c)
file(GLOB_RECURSE TEST_CASE_CONFIGS t*/.clang-uml)
file(GLOB_RECURSE TEST_CONFIG_YMLS test_config_data/*.yml)

set(TEST_CASES_REQUIRING_CXX20 t00056 t00058)

set(CLANG_UML_TEST_LIBRARIES
        clang-umllib
        ${YAML_CPP_LIBRARIES}
        ${LIBTOOLING_LIBS}
        Threads::Threads)

if(MSVC)
    list(APPEND CLANG_UML_TEST_LIBRARIES "Version.lib")
endif(MSVC)

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_20 SUPPORTS_CXX_STD_20)

if(SUPPORTS_CXX_STD_20 EQUAL -1
        OR ${LLVM_PACKAGE_VERSION} VERSION_LESS "14.0")
    set(ENABLE_CXX_STD_20_TEST_CASES 0)
    foreach(CXX20_TC ${TEST_CASES_REQUIRING_CXX20})
        list(FILTER TEST_CASE_SOURCES
                EXCLUDE
                REGEX ".*${CXX20_TC}.*")
        list(FILTER TEST_CASE_CONFIGS
                EXCLUDE
                REGEX ".*${CXX20_TC}.*")
    endforeach()
else()
    set(ENABLE_CXX_STD_20_TEST_CASES 1)
endif()

set(TEST_CASES
        test_util
        test_model
        test_cases
        test_decorator_parser
        test_config
        test_filters
        test_thread_pool_executor)

foreach(TEST_NAME ${TEST_CASES})
    add_executable(${TEST_NAME}
            ${TEST_NAME}.cc
            $<$<STREQUAL:${TEST_NAME},test_cases>:${TEST_CASE_SOURCES}>
            catch.h)
    target_compile_features(${TEST_NAME} PRIVATE
            $<IF:${ENABLE_CXX_STD_20_TEST_CASES},cxx_std_20,cxx_std_17>)
    target_compile_definitions(${TEST_NAME} PRIVATE
            $<$<EQUAL:${ENABLE_CXX_STD_20_TEST_CASES},1>:ENABLE_CXX_STD_20_TEST_CASES>)
    target_compile_options(${TEST_NAME} PRIVATE
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -Wno-unused-parameter -Wno-unused-private-field -Wno-unused-variable
            -Wno-attributes -Wno-nonnull -Wno-deprecated-enum-enum-conversion>
            $<$<CXX_COMPILER_ID:MSVC>:/MP /W1 /bigobj /wd4624>)
    target_link_libraries(${TEST_NAME} PRIVATE ${CLANG_UML_TEST_LIBRARIES})
endforeach()

foreach(TEST_CASE_CONFIG ${TEST_CASE_CONFIGS})
    file(RELATIVE_PATH
        TEST_CASE_CONFIG_RELATIVE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${TEST_CASE_CONFIG})
    message(DEBUG ${TEST_CASE_CONFIG_RELATIVE})
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
    message(DEBUG ${TEST_CONFIG_YML_RELATIVE})
    configure_file(
            ${TEST_CONFIG_YML_RELATIVE}
            ${TEST_CONFIG_YML_RELATIVE}
            COPYONLY)
endforeach()

foreach(TEST_NAME ${TEST_CASES})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
