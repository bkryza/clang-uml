
file(GLOB_RECURSE TEST_CASE_SOURCES t*/*.cc t*/*.c t*/src/*.c)
file(GLOB_RECURSE TEST_CASE_MODULE_SOURCES t*/src/*.cppm)
file(GLOB_RECURSE TEST_CASE_CONFIGS t*/.clang-uml)
file(GLOB_RECURSE TEST_CONFIG_YMLS test_config_data/*.yml
                                   test_compilation_database_data/*.yml
                                   test_compilation_database_data/*.json)

set(TEST_CASES_REQUIRING_CXX20 t00056 t00058 t00059 t00065 t00069)
set(TEST_CASES_REQUIRING_CXX20_MODULES t00070 t00071 t00072
                                       t30012 t30013 t30014 t30015)

if(ENABLE_CXX_MODULES_TEST_CASES)
    foreach(CXX20_MOD_TC ${TEST_CASES_REQUIRING_CXX20_MODULES})
        list(APPEND TEST_CASES_REQUIRING_CXX20 ${CXX20_MOD_TC})
    endforeach()
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
else()
    foreach(CXX20_MOD_TC ${TEST_CASES_REQUIRING_CXX20_MODULES})
        list(FILTER TEST_CASE_SOURCES
                EXCLUDE
                REGEX ".*${CXX20_MOD_TC}.*")
        list(FILTER TEST_CASE_CONFIGS
                EXCLUDE
                REGEX ".*${CXX20_MOD_TC}.*")

    endforeach()
endif(ENABLE_CXX_MODULES_TEST_CASES)

set(CLANG_UML_TEST_LIBRARIES
        clang-umllib
        ${YAML_CPP_LIBRARIES}
        ${LIBTOOLING_LIBS}
        Threads::Threads)

if(MSVC)
    list(APPEND CLANG_UML_TEST_LIBRARIES "Version.lib")
endif(MSVC)

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_20 SUPPORTS_CXX_STD_20)

message(STATUS "Enabling C++20 test cases")

# Remove test cases which require C++20 if they are not supported here
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

if(APPLE)
    # On Apple Clang <coroutine> header is not available
    list(FILTER TEST_CASE_SOURCES
            EXCLUDE
            REGEX ".*t00069.*")
endif(APPLE)

set(TEST_NAMES
        test_util
        test_model
        test_cases
        test_compilation_database
        test_decorator_parser
        test_config
        test_cli_handler
        test_filters
        test_thread_pool_executor
        test_query_driver_output_extractor)

foreach(TEST_NAME ${TEST_NAMES})
    add_executable(${TEST_NAME})
    if(TEST_NAME STREQUAL "test_cases")
        if(ENABLE_CXX_MODULES_TEST_CASES)
            target_sources(${TEST_NAME} PUBLIC FILE_SET CXX_MODULES FILES
                           ${TEST_CASE_MODULE_SOURCES})
        endif(ENABLE_CXX_MODULES_TEST_CASES)
        target_sources(${TEST_NAME} PUBLIC ${TEST_NAME}.cc
                                           ${TEST_CASE_SOURCES} catch.h)
    else()
        target_sources(${TEST_NAME} PUBLIC ${TEST_NAME}.cc catch.h)
    endif(TEST_NAME STREQUAL "test_cases")

    target_compile_features(${TEST_NAME} PRIVATE
            $<IF:${ENABLE_CXX_STD_20_TEST_CASES},cxx_std_20,cxx_std_17>)
    target_compile_definitions(${TEST_NAME} PRIVATE
            $<$<EQUAL:${ENABLE_CXX_STD_20_TEST_CASES},1>:ENABLE_CXX_STD_20_TEST_CASES>
            $<$<BOOL:${ENABLE_CXX_MODULES_TEST_CASES}>:ENABLE_CXX_MODULES_TEST_CASES>)
    target_compile_options(${TEST_NAME} PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -Wno-unused-parameter -Wno-unused-variable
            -Wno-attributes -Wno-nonnull -Wno-deprecated-enum-enum-conversion
            ${CUSTOM_COMPILE_OPTIONS}>
            $<$<CXX_COMPILER_ID:MSVC>:/MP /MD /W1 /bigobj /wd4624>>)
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

foreach(TEST_NAME ${TEST_NAMES})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
