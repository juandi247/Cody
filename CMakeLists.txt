cmake_minimum_required(VERSION 3.22.1)
project(Cody)

set(CMAKE_C_STANDARD_11)


# set(CMAKE_C_COMPILER clang)


include_directories(${PROJECT_SOURCE_DIR}/include)


set(SOURCES
    src/main.c
    src/master.c
)


add_executable(cody ${SOURCES})
#private bbecause its only for our src files, not dependencies
    target_compile_options(cody PRIVATE -Wall -Wextra -O2)