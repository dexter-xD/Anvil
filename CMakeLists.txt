cmake_minimum_required(VERSION 3.10)
project(Anvil VERSION 1.0.0 LANGUAGES C)

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