find_package(Git)

function(setup_git_version)
    if(NOT DEFINED GIT_VERSION)
        if(GIT_EXECUTABLE)
            execute_process(
                    COMMAND ${GIT_EXECUTABLE} describe --tags --always --abbrev=7
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE GIT_VERSION
                    RESULT_VARIABLE GIT_ERROR_CODE
                    OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif(GIT_EXECUTABLE)
    endif(NOT DEFINED GIT_VERSION)

    if(NOT DEFINED GIT_VERSION OR "${GIT_VERSION}" STREQUAL "")
        set(GIT_VERSION "0.0.0-unknown")
    endif(NOT DEFINED GIT_VERSION OR "${GIT_VERSION}" STREQUAL "")

    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.(.+)"
            GIT_VERSION_MATCH ${GIT_VERSION})
    set(GIT_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
    set(GIT_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
    set(GIT_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)
    set(GIT_VERSION ${GIT_VERSION} PARENT_SCOPE)

endfunction()
