message(STATUS "Checking for LLVM and Clang...")

if(LLVM_VERSION STREQUAL "18")
    set(LLVM_VERSION "18.1")
endif()

# If user provided a path to llvm-config executable use it to detect
# LLVM Version and appropriate CMake module path
if(NOT "${LLVM_CONFIG_PATH}" STREQUAL "")
    # Get LLVM prefix
    message(STATUS "Using llvm-config from ${LLVM_CONFIG_PATH}")
    set(LIBCLANG_LLVM_CONFIG_EXECUTABLE ${LLVM_CONFIG_PATH})
    set(LLVM_CONFIG_BINARY ${LLVM_CONFIG_PATH})
    execute_process(COMMAND ${LLVM_CONFIG_PATH} --prefix
            OUTPUT_VARIABLE LLVM_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(LLVM_CMAKE_DIR "${LLVM_PREFIX}/lib/cmake/llvm")

    list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

    # Get LLVM version
    execute_process(COMMAND ${LLVM_CONFIG_PATH} --version
            OUTPUT_VARIABLE LLVM_VERSION_STR OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.(.+)"
            GIT_VERSION_MATCH ${LLVM_VERSION_STR})
    set(LLVM_VERSION ${CMAKE_MATCH_1})
endif()

find_package(LLVM ${LLVM_VERSION} CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
include(AddLLVM)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake from: ${LLVM_CMAKE_DIR}")
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
    if(${LLVM_PACKAGE_VERSION} VERSION_LESS "15.0")
        list(REMOVE_ITEM LIBTOOLING_LIBS clangSupport)
    else()
        list(APPEND LIBTOOLING_LIBS
                LLVMWindowsDriver
                LLVMWindowsManifest)
    endif()
    if(${LLVM_PACKAGE_VERSION} VERSION_GREATER_EQUAL "16.0")
        list(APPEND LIBTOOLING_LIBS clangASTMatchers)
    endif()
    if(MSVC)
        if(${LLVM_PACKAGE_VERSION} VERSION_GREATER_EQUAL "18.1")
            list(APPEND LIBTOOLING_LIBS clangAPINotes)
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
