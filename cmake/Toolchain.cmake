set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

# Replace with your path and triplet
set(HOST i686-elf)
set(CROSS_PATH /home/anant/dev/projs/barebones/opt/cross/bin)

set(CMAKE_SYSROOT ${CMAKE_SOURCE_DIR}/sysroot)

set(CMAKE_C_COMPILER ${CROSS_PATH}/${HOST}-gcc)
set(CMAKE_ASM_COMPILER ${CROSS_PATH}/${HOST}-gcc)
set(CMAKE_AR ${CROSS_PATH}/${HOST}-ar)
set(CMAKE_OBJCOPY ${CROSS_PATH}/${HOST}-objcopy)

set(CMAKE_C_COMPILER_WORKS TRUE)  # <- Bypass test
set(CMAKE_ASM_COMPILER_WORKS TRUE)

# Freestanding flags
set(CMAKE_C_FLAGS "-ffreestanding -fno-pie -fno-stack-protector -nostdlib -mgeneral-regs-only -Wall -Wextra")
