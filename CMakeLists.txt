cmake_minimum_required(VERSION 3.31)

# project name init
project(
    taskxr
    VERSION 1.0
    DESCRIPTION "TaskXr - Task Executor. A simple cron-like utility to schedule tasks with multiple job files."
    LANGUAGES C
)

# setting default build type
set(CMAKE_C_STANDARD 11)
set(default_buid_type "Debug")
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib) # for libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin) # for binaries

# finding all headers and sources
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/inc/*.h)

# add subproject
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/vita) # build vita submodule

# include directories
include_directories(${PROJECT_SOURCE_DIR}/inc/) # project headers
include_directories(${PROJECT_SOURCE_DIR}/third_party/vita/inc/) # vita submodule headers

# building library/binary
# add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS}) # for libraries
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS}) # for binaries

# linking 
target_link_libraries(${PROJECT_NAME} vita) # link vita static library
