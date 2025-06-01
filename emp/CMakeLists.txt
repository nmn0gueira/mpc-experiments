cmake_minimum_required(VERSION 3.14)
project (emp-test)
set(NAME "emp-test")

#set(CMAKE_FOLDER ./)
#include(${CMAKE_FOLDER}/cmake/base.cmake)

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-sh2pc REQUIRED)
include_directories(${EMP-SH2PC_INCLUDE_DIRS})

add_subdirectory(src)