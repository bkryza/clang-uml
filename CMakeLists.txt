cmake_minimum_required(VERSION 3.12)

#
# Project name
#
project(clang-uml)

#
# CMake standard defines
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_FIND_DEBUG_MODE OFF)

#
# clang-uml custom defines
#
set(CLANG_UML_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CLANG_UML_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(UML_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/uml)

#
# CMake build options
#
option(LINK_LLVM_SHARED "Should LLVM be linked using shared libraries or statically" ON)
set(LLVM_VERSION CACHE STRING "Path to custom llvm-config executable")
set(GIT_VERSION "0.3.0" CACHE STRING "clang-uml version")

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
endif(LINK_LLVM_SHARED)

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
	    PATHS ${YAML_CPP_INCLUDE_DIR} /usr/local/include/yaml-cpp)
    find_library(YAML_CPP_LIBRARY
            NAMES yaml-cpp
	    PATHS ${YAML_CPP_LIBRARIES} /usr/local/lib)
    set(YAML_CPP_LIBRARY_DIR /usr/local/lib)
endif()
else(APPLE)
    find_package(yaml-cpp REQUIRED)
endif(APPLE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -std=c++17 -Wno-unused-parameter -Wno-unused-private-field")

link_directories(${LLVM_LIBRARY_DIR} ${YAML_CPP_LIBRARY_DIR})

#
# Setup thirdparty sources
#
set(THIRDPARTY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/thirdparty/)

#
# Configure clang-uml executable version
#
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/src/version)
configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/src/version/version.h)

#
# Handle various compiler quirks
#
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(COMMAND gcc --print-file-name=include
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    OUTPUT_VARIABLE GCC_STDDEF_INCLUDE)
    message(STATUS "FOUND GCC STDDEF INCLUDE ${GCC_STDDEF_INCLUDE}")
    include_directories(${GCC_STDDEF_INCLUDE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${GCC_STDDEF_INCLUDE}")
endif()

message(STATUS "Using CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

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


#
# Generate source list dynamically
#
file(GLOB_RECURSE SOURCES src/*.cc include/*.h)
set(MAIN_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN_SOURCE_FILE})


#
# Define library target for linking with test cases and output executable
#
add_library(clang-umllib OBJECT ${SOURCES})

#
# Define the target executable clang-uml
#
add_executable(clang-uml ${MAIN_SOURCE_FILE})
target_link_libraries(clang-uml
        ${YAML_CPP_LIBRARIES}
        ${LIBTOOLING_LIBS}
        clang-umllib
        Threads::Threads)

target_compile_features(clang-uml PRIVATE cxx_std_17)

#
# Setup install options
#
include(GNUInstallDirs)

install(TARGETS clang-uml DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES LICENSE.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

#
# Enable testing via CTest
#
option(BUILD_TESTS "" ON)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif(BUILD_TESTS)