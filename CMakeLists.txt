cmake_minimum_required(VERSION 3.12)

project(clang-uml)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CLANG_UML_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CLANG_UML_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(UML_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/uml)

option(LLVM_CONFIG_PATH "Path to custom llvm-config executable")

option(GIT_VERSION "clang-uml version" "0.1.0")

if(LLVM_CONFIG_PATH)
    message(STATUS "Using llvm-config from ${LLVM_CONFIG_PATH}")
    set(LIBCLANG_LLVM_CONFIG_EXECUTABLE ${LLVM_CONFIG_PATH})
    set(LLVM_CONFIG_BINARY ${LLVM_CONFIG_PATH})
endif(LLVM_CONFIG_PATH)

#set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

message(STATUS "Checking for yaml-cpp...")
find_package(yaml-cpp REQUIRED)

message(STATUS "Checking for libclang...")
set(LLVM_PREFERRED_VERSION 14.0.0)
# Add
#   -DLLVM_CONFIG_PATH=/path/to/llvm-config
# to use custom LLVM version
find_package(LibClang REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -std=c++17 ${LIBCLANG_CXXFLAGS}")

message(STATUS "Using CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# Thirdparty sources
set(THIRDPARTY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/thirdparty/)
add_subdirectory(thirdparty/cppast)

find_package(LLVM REQUIRED CONFIG)
set(CLANG_INCLUDE_DIRS "llvm/clang/include")
set(CLANG_LIBS clang)

# Configure executable version
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/src/version)
configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/src/version/version.h)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(COMMAND gcc --print-file-name=include
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    OUTPUT_VARIABLE GCC_STDDEF_INCLUDE)
    message(STATUS "FOUND GCC STDDEF INCLUDE ${GCC_STDDEF_INCLUDE}")
    include_directories(${GCC_STDDEF_INCLUDE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${GCC_STDDEF_INCLUDE} -Wno-unused-parameter ")
endif()

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
link_directories(${LLVM_LIBRARY_DIR})

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_UML_INSTALL_INCLUDE_DIR})
include_directories(${YAML_CPP_INCLUDE_DIR})
include_directories(${UML_HEADERS_DIR})
include_directories(${THIRDPARTY_HEADERS_DIR})
include_directories(${THIRDPARTY_HEADERS_DIR}/cppast/include)
include_directories(${THIRDPARTY_HEADERS_DIR}/cppast/external/type_safe/include)
include_directories(${THIRDPARTY_HEADERS_DIR}/cppast/external/type_safe/external/debug_assert)
include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_BINARY_DIR}/src/version)


file(GLOB_RECURSE SOURCES src/*.cc include/*.h)
set(MAIN_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN_SOURCE_FILE})

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
        LLVMSupport
        )

add_library(clang-umllib OBJECT ${SOURCES})

add_executable(clang-uml ${MAIN_SOURCE_FILE})
target_link_libraries(clang-uml
        ${LIBCLANG_LIBRARIES}
        ${YAML_CPP_LIBRARIES}
        ${LIBTOOLING_LIBS}
        cppast
        clang-umllib
        Threads::Threads)

target_compile_features(clang-uml PRIVATE cxx_std_17)

include(GNUInstallDirs)

install(TARGETS clang-uml DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES LICENSE.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

# Enable testing via CTest
enable_testing()
add_subdirectory(tests)
