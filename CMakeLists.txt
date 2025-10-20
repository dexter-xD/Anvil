cmake_minimum_required(VERSION 3.10)
project(Anvil VERSION 1.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

option(COPY_TO_EXAMPLE "Copy executable to example folder after build" ON)

include_directories(include)
set(SOURCES
    src/main.c
    src/config_parser.c
    src/file_utils.c
    src/makefile_generator.c
    src/string_utils.c
    src/watch_system.c
)

add_executable(anvil ${SOURCES})

# Optionally copy executable to example folder after successful build
if(COPY_TO_EXAMPLE)
    add_custom_command(TARGET anvil POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:anvil> ${CMAKE_SOURCE_DIR}/example/
        COMMENT "Copying anvil executable to example folder..."
    )
endif()


install(TARGETS anvil DESTINATION bin)

# Install documentation
install(FILES README.md DESTINATION share/doc/anvil)
install(FILES LICENSE DESTINATION share/doc/anvil)

# CPack configuration for Debian package
set(CPACK_GENERATOR "DEB")

# Allow overriding package name and version
if(NOT DEFINED PACKAGE_NAME)
    set(PACKAGE_NAME "anvil")
endif()
if(NOT DEFINED PACKAGE_VERSION)
    set(PACKAGE_VERSION "1.1.0")
endif()

set(CPACK_PACKAGE_NAME "${PACKAGE_NAME}")
set(CPACK_PACKAGE_VERSION "${PACKAGE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fast C build system with watch mode")
set(CPACK_PACKAGE_DESCRIPTION "Anvil is a lightweight, fast build system for C projects that generates
Makefiles from simple configuration files. Features include:
 * Simple configuration syntax
 * Automatic source file discovery with wildcards  
 * Watch mode for automatic rebuilds
 * Colored output for better readability
 * Cross-platform support")

set(CPACK_PACKAGE_CONTACT "Dexter <dexter@example.com>")
set(CPACK_PACKAGE_VENDOR "Dexter")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/dexter/anvil")

# Debian specific settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dexter <dexter@example.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "build-essential")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

# Package file name
set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_NAME}_${PACKAGE_VERSION}_amd64")

include(CPack)