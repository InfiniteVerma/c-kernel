file(GLOB STDIO_SRC stdio/*.c)
file(GLOB STDLIB_SRC stdlib/*.c)
file(GLOB STRING_SRC string/*.c)

set(LIBC_SOURCES
    ${STDIO_SRC}
    ${STDLIB_SRC}
    ${STRING_SRC}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include       # for <stdio.h>, etc.
    ${CMAKE_SOURCE_DIR}/kernel/include        # for <kernel/panic.h>
)

add_library(libc STATIC ${LIBC_SOURCES})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_SYSROOT}/usr/include
        FILES_MATCHING PATTERN "*.h")

install(TARGETS libc DESTINATION ${CMAKE_SYSROOT}/usr/lib)
