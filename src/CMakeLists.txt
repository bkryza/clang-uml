#
# Configure clang-uml executable version
#
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/src/version)
configure_file(${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${PROJECT_BINARY_DIR}/src/version/version.h)

#
# Generate source list dynamically
#
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cc)
set(MAIN_SOURCE_FILE ${CMAKE_CURRENT_LIST_DIR}/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN_SOURCE_FILE})

#
# Define library target for linking with test cases and output executable
#
if(MSVC)
    add_library(clang-umllib STATIC ${SOURCES})
    set(MSVC_LIBRARIES "version")
else(MSVC)
    add_library(clang-umllib OBJECT ${SOURCES})
endif(MSVC)

target_compile_features(clang-umllib INTERFACE cxx_std_17)
target_compile_options(clang-umllib PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra -Wno-unused-parameter
        -Wno-deprecated-declarations ${CUSTOM_COMPILE_OPTIONS}>
        $<$<CXX_COMPILER_ID:MSVC>:/MP /MD /W1 /bigobj /wd4291 /wd4624 /wd4244>)
target_compile_definitions(clang-umllib PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:
        -DLLVM_FORCE_USE_OLD_TOOLCHAIN
        -DTERMCOLOR_USE_WINDOWS_API=1
        -DTERMCOLOR_TARGET_WINDOWS=1>)

#
# Define the target executable clang-uml
#
add_executable(clang-uml ${MAIN_SOURCE_FILE})

target_compile_features(clang-uml PUBLIC cxx_std_17)
target_compile_options(clang-uml PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra -Wno-unused-parameter
        -Wno-deprecated-declarations ${CUSTOM_COMPILE_OPTIONS}>
        $<$<CXX_COMPILER_ID:MSVC>:/MP /MD /W1 /bigobj /wd4291 /wd4624 /wd4244>)
target_compile_definitions(clang-uml PRIVATE
        ${ENABLE_BACKWARD_CPP})
target_link_libraries(clang-uml
        ${YAML_CPP_LIBRARIES}
        ${LIBTOOLING_LIBS}
        ${MSVC_LIBRARIES}
        ${BACKWARD_CPP_LIBRARIES}
        clang-umllib
        Threads::Threads)

#
# Setup install options
#
include(GNUInstallDirs)

install(TARGETS clang-uml DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${PROJECT_SOURCE_DIR}/LICENSE.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES ${PROJECT_SOURCE_DIR}/README.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES ${PROJECT_SOURCE_DIR}/AUTHORS.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES ${PROJECT_SOURCE_DIR}/CHANGELOG.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})

#
# Setup installer
#
set(CPACK_PACKAGE_NAME "clang-uml")
set(CPACK_PACKAGE_VENDOR "Bartek Kryza <bkryza@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "clang-uml - C++ UML diagram generator based on Clang")
set(CPACK_PACKAGE_VERSION "${GIT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${GIT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GIT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${GIT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "clang-uml")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md)

if(MSVC)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_DISPLAY_NAME "clang-uml")
    set(CPACK_NSIS_HELP_LINK "https://github.com/bkryza/clang-uml")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/bkryza/clang-uml")
    set(CPACK_NSIS_CONTACT "Bartek Kryza <bkryza@gmail.com>")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif(MSVC)

include(CPack)
