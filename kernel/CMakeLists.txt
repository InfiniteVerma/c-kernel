# kernel/CMakeLists.txt
enable_language(ASM)

# Collect source files
file(GLOB_RECURSE KERNEL_C_SOURCES
    "${CMAKE_SOURCE_DIR}/kernel/*.c"
    "${CMAKE_SOURCE_DIR}/kernel/io/*.c"
    "${CMAKE_SOURCE_DIR}/arch/i386/tty.c"
)

set(KERNEL_ASM_SOURCES
    ${CMAKE_SOURCE_DIR}/kernel/arch/i386/boot.S
    ${CMAKE_SOURCE_DIR}/kernel/arch/i386/crti.S
    ${CMAKE_SOURCE_DIR}/kernel/arch/i386/crtn.S
)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/arch/i386/linker.ld)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/arch/i386
    ${CMAKE_SOURCE_DIR}/libc/include
)

# Final ELF output
add_executable(kernel.elf ${KERNEL_C_SOURCES} ${KERNEL_ASM_SOURCES})

# Linker flags
set_target_properties(kernel.elf PROPERTIES
    LINK_FLAGS "-T ${LINKER_SCRIPT} -nostdlib -Wl,--build-id=none -Wl,--no-dynamic-linker -Wl,-N"
)

# Generate flat binary (kernel.bin) for booting
add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary kernel.elf kernel.bin
    COMMENT "Generating kernel.bin from kernel.elf"
)

# Install kernel.bin into sysroot/boot
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin DESTINATION ${CMAKE_SYSROOT}/boot)

target_link_libraries(kernel.elf PRIVATE libc)

# Generate myos.iso using iso.sh after kernel.bin is built
add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Generating bootable ISO..."
    COMMAND ${CMAKE_SOURCE_DIR}/iso.sh
    BYPRODUCTS ${CMAKE_BINARY_DIR}/myos.iso
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running iso.sh to create myos.iso"
    VERBATIM
)
