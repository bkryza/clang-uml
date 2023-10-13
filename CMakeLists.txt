cmake_minimum_required(VERSION 3.12)

#
# Project name
#
project(clang-uml C CXX)

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

#
# Setup version string
#
include(GitVersion)
setup_git_version()
message(STATUS "clang-uml version: "
               "${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}.${GIT_VERSION_PATCH}")

#
# Setup LLVM
#
message(STATUS "Checking for LLVM and Clang...")
if(LLVM_PREFIX)
    message(STATUS "Using llvm-config from ${LLVM_CONFIG_PATH}")
    set(LIBCLANG_LLVM_CONFIG_EXECUTABLE ${LLVM_CONFIG_PATH})
    set(LLVM_CONFIG_BINARY ${LLVM_CONFIG_PATH})
endif(LLVM_PREFIX)

find_package(LLVM ${LLVM_VERSION} CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(AddLLVM)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM library dir: ${LLVM_LIBRARY_DIR}")

if(MSVC)
    # LLVM_BUILD_LLVM_DYLIB is not available on Windows
    set(LINK_LLVM_SHARED NO)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

if(LINK_LLVM_SHARED)
    set(LIBTOOLING_LIBS clang-cpp LLVM)
else(LINK_LLVM_SHARED)
    set(LIBTOOLING_LIBS
        clangLex
        clangFrontend
        clangSerialization
        clangDriver
        clangParse
        clangSema
        clangSupport
        clangAnalysis
        clangAST
        clangBasic
        clangEdit
        clangLex
        clangTooling
        LLVMipo
        LLVMScalarOpts
        LLVMInstCombine
        LLVMTransformUtils
        LLVMAnalysis
        LLVMTarget
        LLVMOption
        LLVMMCParser
        LLVMMC
        LLVMObject
        LLVMBitReader
        LLVMCore
        LLVMSupport)
    if(MSVC)
        if(${LLVM_PACKAGE_VERSION} VERSION_LESS "15.0")
            list(REMOVE_ITEM LIBTOOLING_LIBS clangSupport)
        else()
            list(APPEND LIBTOOLING_LIBS
                        LLVMWindowsDriver
                        LLVMWindowsManifest)
        endif()
    endif(MSVC)
endif(LINK_LLVM_SHARED)

if("${LIBTOOLING_LIBS}" STREQUAL "")
    message(FATAL_ERROR "Failed to find LibTooling libraries!")
else()
    message(STATUS "Found LibTooling libraries: ${LIBTOOLING_LIBS}")
endif()

if(APPLE OR (LLVM_VERSION_MAJOR GREATER_EQUAL 16))
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
            ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

#
# Setup custom compile options depending on various compiler
# and environment quirks
#
if(LLVM_VERSION_MAJOR GREATER_EQUAL 17)
    set(CUSTOM_COMPILE_OPTIONS "-Wno-class-memaccess")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CUSTOM_COMPILE_OPTIONS
            "${CUSTOM_COMPILE_OPTIONS} -Wno-unused-private-field")
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
            NAMES yaml.h
	        PATHS ${YAML_CPP_INCLUDE_DIR} ${HOMEBREW_PREFIX}/include/yaml-cpp)
        find_library(YAML_CPP_LIBRARY
            NAMES yaml-cpp
            PATHS ${YAML_CPP_LIBRARIES} ${HOMEBREW_PREFIX}/lib)
        set(YAML_CPP_LIBRARY_DIR ${HOMEBREW_PREFIX}/lib)
    endif(PKG_CONFIG_FOUND)
elseif(MSVC)
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
# Setup libdw and libunwind
#
if(LINUX AND (CMAKE_BUILD_TYPE MATCHES Debug))
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DW REQUIRED IMPORTED_TARGET libdw)
    pkg_check_modules(UNWIND REQUIRED IMPORTED_TARGET libunwind)
    set(BACKWARD_CPP_LIBRARIES PkgConfig::DW PkgConfig::UNWIND)
    set(ENABLE_BACKWARD_CPP -DENABLE_BACKWARD_CPP)
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
include_directories(${YAML_CPP_INCLUDE_DIRS})
include_directories(${UML_HEADERS_DIR})
include_directories(${THIRDPARTY_HEADERS_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_BINARY_DIR}/src/version)

add_subdirectory(src)

#
# Enable testing via CTest
#
option(BUILD_TESTS "" ON)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif(BUILD_TESTS)