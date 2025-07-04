file(GLOB_RECURSE TEST_CASE_SOURCES t*/*.cc t*/*.c t*/src/*.c t*/*.cu)
file(GLOB_RECURSE TEST_CASES_OBJC_SOURCES t*/*.m t*/src/*.m)
file(GLOB_RECURSE TEST_CASE_MODULE_SOURCES t*/src/*.cppm)
file(GLOB_RECURSE TEST_CASE_CONFIGS t*/.clang-uml)
file(GLOB_RECURSE TEST_CONFIG_YMLS test_config_data/*.yml
     test_compilation_database_data/*.yml test_compilation_database_data/*.json)

set(TEST_CASES_REQUIRING_CXX20
    t00056
    t00058
    t00059
    t00065
    t00069
    t00074
    t00075
    t00090
    t20064
    t20070
    t20071)
set(TEST_CASES_REQUIRING_CXX20_MODULES
    t00070
    t00071
    t00072
    t30012
    t30013
    t30014
    t30015)
set(TEST_CASES_REQUIRING_CUDA t20049 t20050 t20051)
set(TEST_CASES_REQUIRING_OBJC
    t00084
    t00085
    t00086
    t00088
    t20057
    t20058
    t30016
    t40004)

set(COMPILE_COMMANDS_BUILD_DIR
    ${PROJECT_BINARY_DIR}/tests/test_compilation_database_data/compile_commands_test
)
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/test_compilation_database_data/compile_commands_test/compile_commands.json.in
    ${PROJECT_BINARY_DIR}/tests/test_compilation_database_data/compile_commands_test/compile_commands.json
)
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/test_compilation_database_data/compile_flags_test/compile_flags.txt
    ${PROJECT_BINARY_DIR}/tests/test_compilation_database_data/compile_flags_test/compile_flags.txt
)

if(ENABLE_OBJECTIVE_C_TEST_CASES)
    message(STATUS "Enabling Objective-C test cases")
    if(FETCH_LIBOBJC2)
        include(CPM)
        cpmaddpackage(NAME libobjc2 GITHUB_REPOSITORY bkryza/libobjc2 GIT_TAG
                      fix-cmake-variables)
        set(FETCHED_LIBOBJC2_NAME objc)
    endif()
else()
    message(STATUS "Disabling Objective-C test cases")
endif(ENABLE_OBJECTIVE_C_TEST_CASES)

if(ENABLE_CXX_MODULES_TEST_CASES)
    message(STATUS "Enabling C++ modules test cases")

    foreach(CXX20_MOD_TC ${TEST_CASES_REQUIRING_CXX20_MODULES})
        list(APPEND TEST_CASES_REQUIRING_CXX20 ${CXX20_MOD_TC})
    endforeach()
    # Remove the module test cases *.cc sources as they will be added
    # to test_cases_mod target
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
    foreach(CXX20_MOD_TC ${TEST_CASES_REQUIRING_CXX20_MODULES})
        list(FILTER TEST_CASE_SOURCES EXCLUDE REGEX ".*${CXX20_MOD_TC}.*")
    endforeach()
else()
    message(STATUS "Disabling C++ modules test cases")

    foreach(CXX20_MOD_TC ${TEST_CASES_REQUIRING_CXX20_MODULES})
        list(FILTER TEST_CASE_SOURCES EXCLUDE REGEX ".*${CXX20_MOD_TC}.*")
        list(FILTER TEST_CASE_CONFIGS EXCLUDE REGEX ".*${CXX20_MOD_TC}.*")
    endforeach()
endif(ENABLE_CXX_MODULES_TEST_CASES)

if(NOT ENABLE_CUDA_TEST_CASES)
    message(STATUS "Disabling CUDA test cases")
    foreach(CUDA_TC ${TEST_CASES_REQUIRING_CUDA})
        list(FILTER TEST_CASE_SOURCES EXCLUDE REGEX ".*${CUDA_TC}.*")
        list(FILTER TEST_CASE_CONFIGS EXCLUDE REGEX ".*${CUDA_TC}.*")
    endforeach()
else()
    message(STATUS "Enabling CUDA test cases")
endif(NOT ENABLE_CUDA_TEST_CASES)

set(CLANG_UML_TEST_LIBRARIES clang-umllib ${YAML_CPP_LIBRARIES}
                             ${LIBTOOLING_LIBS} ${LIBUNWIND_LIBRARY} Threads::Threads)

if(MSVC)
    list(APPEND CLANG_UML_TEST_LIBRARIES "Version.lib")
else(MSVC)
    list(APPEND CLANG_UML_TEST_LIBRARIES pugixml::pugixml)
endif(MSVC)

list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_20 SUPPORTS_CXX_STD_20)

# Remove test cases which require C++20 if they are not supported here
if(SUPPORTS_CXX_STD_20 EQUAL -1 OR ${LLVM_PACKAGE_VERSION} VERSION_LESS "14.0")
    set(ENABLE_CXX_STD_20_TEST_CASES 0)
    foreach(CXX20_TC ${TEST_CASES_REQUIRING_CXX20})
        list(FILTER TEST_CASE_SOURCES EXCLUDE REGEX ".*${CXX20_TC}.*")
        list(FILTER TEST_CASE_CONFIGS EXCLUDE REGEX ".*${CXX20_TC}.*")
    endforeach()
    message(STATUS "Disabling C++20 test cases")
else()
    set(ENABLE_CXX_STD_20_TEST_CASES 1)
    message(STATUS "Enabling C++20 test cases")
endif()

set(TEST_NAMES
    test_types
    test_util
    test_model
    test_cases
    test_compilation_database
    test_decorator_parser
    test_config
    test_cli_handler
    test_filters
    test_filters_advanced
    test_levenshtein
    test_nested_trait
    test_thread_pool_executor
    test_query_driver_output_extractor
    test_progress_indicator)

if(ENABLE_BENCHMARKS)
    message(STATUS "Enabling microbenchmarks")
    list(APPEND TEST_NAMES test_benchmarks)
endif(ENABLE_BENCHMARKS)

macro(generate_test_cases_with_paths input_list output_list)
    set(${output_list}) # Initialize the output list
    foreach(test_case ${${input_list}})
        list(APPEND ${output_list}
             "${CMAKE_CURRENT_SOURCE_DIR}/${test_case}/${test_case}.cc")
    endforeach()
endmacro()

generate_test_cases_with_paths(TEST_CASES_REQUIRING_CXX20_MODULES
                               TEST_CASES_REQUIRING_CXX20_MODULES_PATHS)
foreach(TEST_NAME ${TEST_NAMES})
    add_executable(${TEST_NAME})
    if(TEST_NAME STREQUAL "test_cases")
        if(ENABLE_CXX_MODULES_TEST_CASES)
            add_library(test_cases_mod)
            set_target_properties(test_cases_mod PROPERTIES CXX_SCAN_FOR_MODULES
                                                            ON)
            target_sources(
                test_cases_mod
                PUBLIC ${TEST_CASES_REQUIRING_CXX20_MODULES_PATHS}
                PUBLIC FILE_SET CXX_MODULES FILES ${TEST_CASE_MODULE_SOURCES})
            target_compile_features(test_cases_mod PRIVATE cxx_std_20)
        endif(ENABLE_CXX_MODULES_TEST_CASES)
        if(ENABLE_OBJECTIVE_C_TEST_CASES)
            # We need to link the objective test cases sources to a separate
            # ObjC library than the rest of C++ test cases
            add_library(test_cases_objc)
            target_compile_options(
                test_cases_objc
                PRIVATE
                    "SHELL:-isystem ${LLVM_INCLUDE_DIR}/c++/v1"
                    "SHELL:-isystem ${LLVM_LIBRARY_DIR}/clang/${LLVM_VERSION_MAJOR}/include"
            )
            if(NOT APPLE)
                target_compile_options(test_cases_objc
                                       PRIVATE ${GNUSTEP_CFLAGS})
                target_link_libraries(
                    test_cases_objc PRIVATE ${GNUSTEP_LDFLAGS}
                                            ${FETCHED_LIBOBJC2_NAME})
            endif(NOT APPLE)
            target_sources(test_cases_objc PUBLIC ${TEST_CASES_OBJC_SOURCES})
        endif(ENABLE_OBJECTIVE_C_TEST_CASES)
        target_sources(${TEST_NAME} PUBLIC ${TEST_NAME}.cc
                test_case_utils/test_case_utils.cc ${TEST_CASE_SOURCES})
        target_include_directories(${TEST_NAME} PUBLIC ${YAML_CPP_INCLUDE_DIR})
    else()
        target_sources(${TEST_NAME} PUBLIC ${TEST_NAME}.cc)
    endif(TEST_NAME STREQUAL "test_cases")

    target_compile_features(
        ${TEST_NAME}
        PRIVATE $<IF:${ENABLE_CXX_STD_20_TEST_CASES},cxx_std_20,cxx_std_17>)
    target_compile_definitions(
        ${TEST_NAME}
        PRIVATE
            $<$<EQUAL:${ENABLE_CXX_STD_20_TEST_CASES},1>:ENABLE_CXX_STD_20_TEST_CASES>
            $<$<BOOL:${ENABLE_CXX_MODULES_TEST_CASES}>:ENABLE_CXX_MODULES_TEST_CASES>
            $<$<BOOL:${ENABLE_CUDA_TEST_CASES}>:ENABLE_CUDA_TEST_CASES>
            $<$<BOOL:${ENABLE_OBJECTIVE_C_TEST_CASES}>:ENABLE_OBJECTIVE_C_TEST_CASES>
    )
    target_compile_options(
        ${TEST_NAME}
        PRIVATE $<$<COMPILE_LANGUAGE:CXX>:
                $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
                -Wno-unused-parameter
                -Wno-unused-variable
                -Wno-attributes
                -Wno-nonnull
                -Wno-deprecated-enum-enum-conversion
                ${CUSTOM_COMPILE_OPTIONS}>
                $<$<CXX_COMPILER_ID:MSVC>:/MP
                /MD
                /W1
                /bigobj
                /wd4624>>)
    target_link_libraries(${TEST_NAME} PRIVATE ${CLANG_UML_TEST_LIBRARIES})
endforeach()

foreach(TEST_CASE_CONFIG ${TEST_CASE_CONFIGS})
    file(RELATIVE_PATH TEST_CASE_CONFIG_RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         ${TEST_CASE_CONFIG})
    message(DEBUG ${TEST_CASE_CONFIG_RELATIVE})
    configure_file(${TEST_CASE_CONFIG_RELATIVE} ${TEST_CASE_CONFIG_RELATIVE}
                   COPYONLY)
endforeach()

foreach(TEST_CONFIG_YML ${TEST_CONFIG_YMLS})
    file(RELATIVE_PATH TEST_CONFIG_YML_RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         ${TEST_CONFIG_YML})
    message(DEBUG ${TEST_CONFIG_YML_RELATIVE})
    configure_file(${TEST_CONFIG_YML_RELATIVE} ${TEST_CONFIG_YML_RELATIVE}
                   COPYONLY)
endforeach()

foreach(TEST_NAME ${TEST_NAMES})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
