cmake_minimum_required(VERSION 3.15)
project(MyKernel LANGUAGES C)

# Paths
#include(cmake/Macros.cmake)
set(SYSROOT ${CMAKE_SOURCE_DIR}/sysroot)
set(CMAKE_INSTALL_PREFIX /usr)
set(INCLUDE_INSTALL_DIR include)
set(LIB_INSTALL_DIR lib)
set(EXEC_INSTALL_DIR bin)

# Add projects
add_subdirectory(libc)
add_subdirectory(kernel)

# ISO target
#add_custom_target(iso
#    COMMAND ${CMAKE_SOURCE_DIR}/scripts/iso.sh
#    DEPENDS kernel
#    COMMENT "Creating bootable ISO"
#)
#
## Run target (QEMU)
#add_custom_target(run
#    COMMAND ${CMAKE_SOURCE_DIR}/scripts/qemu.sh
#    DEPENDS iso
#    COMMENT "Running kernel in QEMU"
#)

#add_custom_target(iso
#    COMMAND ${CMAKE_SOURCE_DIR}/iso.sh
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    COMMENT "Generating bootable ISO with GRUB"
#    VERBATIM
#)
