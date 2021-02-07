cmake_minimum_required(VERSION 3.10)

project(clang-uml)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CLANG_UML_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CLANG_UML_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(UML_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/uml)

message(STATUS "Checking for spdlog...")
find_package(spdlog REQUIRED)

message(STATUS "Checking for yaml-cpp...")
find_package(yaml-cpp REQUIRED)

message(STATUS "Checking for libclang...")
find_package(LibClang REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 ${LIBCLANG_CXXFLAGS}")

# Thirdparty sources
set(THIRDPARTY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/thirdparty/)

find_package(LLVM REQUIRED CONFIG)
set(CLANG_INCLUDE_DIRS "llvm/clang/include")
set(CLANG_LIBS clang)

include_directories(${CLANG_UML_INSTALL_INCLUDE_DIR})
include_directories(${YAML_CPP_INCLUDE_DIR})
include_directories(${UML_HEADERS_DIR})
include_directories(${THIRDPARTY_HEADERS_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src/)

add_subdirectory(tests)

file(GLOB_RECURSE SOURCES src/*.cc include/*.h)
set(MAIN_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN_SOURCE_FILE})

add_executable(clang-uml ${SOURCES} ${MAIN_SOURCE_FILE})
install(TARGETS clang-uml DESTINATION ${CLANG_UML_INSTALL_BIN_DIR})
target_link_libraries(clang-uml ${LIBCLANG_LIBRARIES} ${YAML_CPP_LIBRARIES} spdlog::spdlog)

install(
        FILES
        # add include after DESTINATION, then it works
        DESTINATION include ${CMAKE_INSTALL_INCLUDEDIR}
)
