cmake_minimum_required(VERSION 3.16)

#
# Project name
#
project(clang-uml C CXX)

cmake_policy(SET CMP0054 NEW)

#
# CMake standard defines
#
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_FIND_DEBUG_MODE OFF)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

#
# C++ setup
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# clang-uml custom defines
#
set(CLANG_UML_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CLANG_UML_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(UML_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/uml)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
if(APPLE)
    if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm"
            OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
        set(HOMEBREW_PREFIX "/opt/homebrew")
    else()
        set(HOMEBREW_PREFIX "/usr/local")
    endif()
endif(APPLE)

#
# CMake build options
#
option(LINK_LLVM_SHARED "Should LLVM be linked using shared libraries" ON)
set(LLVM_VERSION CACHE STRING "Major LLVM version to use (e.g. 15)")
set(LLVM_CONFIG_PATH CACHE STRING "Path to llvm-config binary")
set(CMAKE_PREFIX CACHE STRING "Path to custom cmake modules")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_PREFIX}")

#
# Setup version string
#
include(GitVersion)
setup_git_version()
message(STATUS "clang-uml version: "
               "${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}.${GIT_VERSION_PATCH}")

#
# Setup coverage
#
option(CODE_COVERAGE "" OFF)
if(CODE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g --coverage -fno-inline")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g --coverage -fno-inline")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif(CODE_COVERAGE)

#
# Setup Address Sanitizer
#
option(ADDRESS_SANITIZER "" OFF)
if(ADDRESS_SANITIZER)
    set(CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} \
            -fno-omit-frame-pointer \
            -fsanitize=address \
            -fsanitize-blacklist=${CMAKE_CURRENT_SOURCE_DIR}/.sanitize-blacklist")
    set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} \
            -fno-omit-frame-pointer \
            -fsanitize=address \
            -fsanitize-blacklist=${CMAKE_CURRENT_SOURCE_DIR}/.sanitize-blacklist")
endif(ADDRESS_SANITIZER)

#
# Setup LLVM
#
include(LLVMSetup)

#
# Setup custom compile options depending on various compiler
# and environment quirks
#
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13.0")
        # Workaround over Wdangling-reference false positives in libfmt
        set(CUSTOM_COMPILE_OPTIONS ${CUSTOM_COMPILE_OPTIONS} -Wno-dangling-reference)
    endif()
endif()

if(LLVM_VERSION_MAJOR GREATER_EQUAL 17)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CUSTOM_COMPILE_OPTIONS ${CUSTOM_COMPILE_OPTIONS} -Wno-class-memaccess)
    endif()
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CUSTOM_COMPILE_OPTIONS
            ${CUSTOM_COMPILE_OPTIONS} -Wno-unused-private-field)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19)
        set(CUSTOM_COMPILE_OPTIONS ${CUSTOM_COMPILE_OPTIONS} -Wno-missing-template-arg-list-after-template-kw)
    endif()
    if(APPLE)
        if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
            # Use LLVM bundled unwind on Apple Silicon to fix segfaults on exception throw
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${HOMEBREW_PREFIX}/opt/llvm/lib/unwind -lunwind")
        endif()
    endif(APPLE)
endif()

#
# Setup threads library
#
find_package(Threads REQUIRED)

#
# Setup yaml-cpp
#
message(STATUS "Checking for yaml-cpp...")
if(APPLE)
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(YAML_CPP yaml-cpp)
        find_path(YAML_CPP_INCLUDE_DIR
            NAMES yaml-cpp/yaml.h
	        PATHS ${YAML_CPP_INCLUDE_DIR} ${HOMEBREW_PREFIX}/include)
        find_library(YAML_CPP_LIBRARY
            NAMES yaml-cpp
            PATHS ${YAML_CPP_LIBRARIES} ${HOMEBREW_PREFIX}/lib)
        set(YAML_CPP_LIBRARY_DIR ${HOMEBREW_PREFIX}/lib)
    endif(PKG_CONFIG_FOUND)
elseif(MSVC)
    find_package(yaml-cpp REQUIRED)
    set(YAML_CPP_LIBRARIES "yaml-cpp")
else()
    find_package(yaml-cpp REQUIRED)
endif()

if("${YAML_CPP_LIBRARIES}" STREQUAL "")
    message(FATAL_ERROR "Failed to find yaml-cpp library!")
else()
    message(STATUS "Found yaml-cpp libraries: ${YAML_CPP_LIBRARIES}")
endif()

link_directories(${LLVM_LIBRARY_DIR} ${YAML_CPP_LIBRARY_DIR})

#
# Setup backward-cpp, libdw and libunwind
#
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CLANG_UML_ENABLE_BACKTRACE_DEFAULT ON)
else()
    set(CLANG_UML_ENABLE_BACKTRACE_DEFAULT OFF)
endif()
option(CLANG_UML_ENABLE_BACKTRACE "Enable backtrace on segfault"
        ${CLANG_UML_ENABLE_BACKTRACE_DEFAULT})

if(LINUX AND CLANG_UML_ENABLE_BACKTRACE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DW REQUIRED IMPORTED_TARGET libdw)
    pkg_check_modules(UNWIND REQUIRED IMPORTED_TARGET libunwind)
    set(BACKWARD_CPP_LIBRARIES PkgConfig::DW PkgConfig::UNWIND)
    set(ENABLE_BACKWARD_CPP -DENABLE_BACKWARD_CPP)
    message(STATUS "Enabling backward-cpp")
else()
    message(STATUS "Disabling backward-cpp")
endif()

#
# Setup thirdparty sources
#
set(THIRDPARTY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/thirdparty/)

#
# Setup include directories
#
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_UML_INSTALL_INCLUDE_DIR})
include_directories(${UML_HEADERS_DIR})
include_directories(${THIRDPARTY_HEADERS_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_BINARY_DIR}/src/version)
include_directories(${YAML_CPP_INCLUDE_DIR})

add_subdirectory(src)

#
# Enable testing via CTest
#
option(BUILD_TESTS "" ON)
option(ENABLE_CXX_MODULES_TEST_CASES "" OFF)
option(ENABLE_CUDA_TEST_CASES "" OFF)
option(ENABLE_OBJECTIVE_C_TEST_CASES "" OFF)
option(FETCH_LIBOBJC2 "" OFF)

#
# Setup CUDA if available
#
if(ENABLE_CUDA_TEST_CASES)
    include(CheckLanguage)
    check_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_ARCHITECTURES 75)
    enable_language(CUDA)
endif(ENABLE_CUDA_TEST_CASES)

if(ENABLE_OBJECTIVE_C_TEST_CASES)
    enable_language(OBJC)
    if(UNIX AND NOT APPLE)
      find_package(GNUstep REQUIRED)
    endif()
endif(ENABLE_OBJECTIVE_C_TEST_CASES)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(BUILD_TESTS)
