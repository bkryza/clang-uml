cmake_minimum_required(VERSION 3.10)

project(clang-uml)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CLANG_UML_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CLANG_UML_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(UML_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/uml)

option(LLVM_CONFIG_PATH "Path to custom llvm-config executable")

if(LLVM_CONFIG_PATH)
    message(STATUS "Using llvm-config from ${LLVM_CONFIG_PATH}")
    set(LIBCLANG_LLVM_CONFIG_EXECUTABLE ${LLVM_CONFIG_PATH})
    set(LLVM_CONFIG_BINARY ${LLVM_CONFIG_PATH})
endif(LLVM_CONFIG_PATH)

message(STATUS "Checking for fmt...")
find_package(fmt REQUIRED)

message(STATUS "Checking for spdlog...")
find_package(spdlog REQUIRED)
if(APPLE)
    get_target_property(SPDLOG_INCLUDE_DIRS spdlog::spdlog_header_only INTERFACE_INCLUDE_DIRECTORIES)
else(APPLE)
    get_target_property(SPDLOG_INCLUDE_DIRS spdlog::spdlog INTERFACE_INCLUDE_DIRECTORIES)
    add_definitions(-DSPDLOG_FMT_EXTERNAL)
endif(APPLE)


message(STATUS "Checking for yaml-cpp...")
find_package(yaml-cpp REQUIRED)

message(STATUS "Checking for libclang...")
set(LLVM_PREFERRED_VERSION 12.0.0)
# Add
#   -DLLVM_CONFIG_PATH=/path/to/llvm-config
# to use custom LLVM version
find_package(LibClang REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 ${LIBCLANG_CXXFLAGS}")
message(STATUS "Using CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# Thirdparty sources
set(THIRDPARTY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/thirdparty/)
add_subdirectory(thirdparty/cppast)

find_package(LLVM REQUIRED CONFIG)
set(CLANG_INCLUDE_DIRS "llvm/clang/include")
set(CLANG_LIBS clang)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(COMMAND gcc --print-file-name=include
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    OUTPUT_VARIABLE GCC_STDDEF_INCLUDE)
    message(STATUS "FOUND GCC STDDEF INCLUDE ${GCC_STDDEF_INCLUDE}")
    include_directories(${GCC_STDDEF_INCLUDE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${GCC_STDDEF_INCLUDE}")
endif()
include_directories(${CLANG_UML_INSTALL_INCLUDE_DIR})
include_directories(${YAML_CPP_INCLUDE_DIR})
include_directories(${SPDLOG_INCLUDE_DIRS})
include_directories(${UML_HEADERS_DIR})
include_directories(${THIRDPARTY_HEADERS_DIR})
include_directories(${THIRDPARTY_HEADERS_DIR}/cppast/include)
include_directories(${THIRDPARTY_HEADERS_DIR}/cppast/external/type_safe/include)
include_directories(${THIRDPARTY_HEADERS_DIR}/cppast/external/type_safe/external/debug_assert)
include_directories(${PROJECT_SOURCE_DIR}/src/)

file(GLOB_RECURSE SOURCES src/*.cc include/*.h)
set(MAIN_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN_SOURCE_FILE})

add_library(clang-umllib OBJECT ${SOURCES})

add_executable(clang-uml ${MAIN_SOURCE_FILE})
install(TARGETS clang-uml DESTINATION ${CLANG_UML_INSTALL_BIN_DIR})
target_link_libraries(clang-uml ${LIBCLANG_LIBRARIES} ${YAML_CPP_LIBRARIES} fmt::fmt cppast clang-umllib)
if(APPLE)
    target_link_libraries(clang-uml spdlog::spdlog_header_only)
else(APPLE)
    target_link_libraries(clang-uml spdlog::spdlog)
endif(APPLE)

target_compile_features(clang-uml PRIVATE cxx_std_17)

install(
        FILES
        # add include after DESTINATION, then it works
        DESTINATION include ${CMAKE_INSTALL_INCLUDEDIR}
)

# Enable testing via CTest
enable_testing()
add_subdirectory(tests)
