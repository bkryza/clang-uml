find_program(GNUSTEP_CONFIG gnustep-config
             HINTS /usr/bin /usr/local/bin /opt/local/bin
)

if (NOT GNUSTEP_CONFIG)
    message(FATAL_ERROR "GNUstep not found. Please install gnustep-base, gnustep-make and libobjc2 packages.")
    return()
endif()

# Get GNUstep CFLAGS (for compilation)
execute_process(
    COMMAND ${GNUSTEP_CONFIG} --objc-flags
    OUTPUT_VARIABLE GNUSTEP_OBJC_FLAGS_STR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(GNUSTEP_OBJC_FLAGS NATIVE_COMMAND ${GNUSTEP_OBJC_FLAGS_STR})

list(REMOVE_ITEM GNUSTEP_OBJC_FLAGS "-MMD")
list(APPEND GNUSTEP_OBJC_FLAGS "-fblocks")
list(APPEND GNUSTEP_OBJC_FLAGS "-fobjc-runtime=gnustep-2.0")
#set(GNUSTEP_OBJC_FLAGS_STR "${GNUSTEP_OBJC_FLAGS_STR}

# Get GNUstep LDFLAGS (for linking)
execute_process(
    COMMAND ${GNUSTEP_CONFIG} --base-libs
    OUTPUT_VARIABLE GNUSTEP_LDFLAGS_STR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(GNUSTEP_LDFLAGS NATIVE_COMMAND ${GNUSTEP_LDFLAGS_STR})

# Export the flags to be used in other CMake files
set(GNUSTEP_FOUND TRUE)
set(GNUSTEP_INCLUDE_DIRS "")
set(GNUSTEP_LIBRARIES "")
set(GNUSTEP_CFLAGS ${GNUSTEP_OBJC_FLAGS})
set(GNUSTEP_LDFLAGS ${GNUSTEP_LDFLAGS})

message(STATUS "GNUstep detected: CFLAGS: ${GNUSTEP_CFLAGS} LDFLAGS: ${GNUSTEP_LDFLAGS}")