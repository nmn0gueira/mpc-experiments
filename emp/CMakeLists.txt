cmake_minimum_required(VERSION 3.14)
project (emp-test)
set(NAME "emp-test")

# Set the correct path to the toolchain file
set(CMAKE_TOOLCHAIN_FILE "c:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Toolchain file")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)
include(${CMAKE_FOLDER}/cmake/enable_rdseed.cmake)
# include(${CMAKE_FOLDER}/cmake/enable_float.cmake)

find_package(emp-sh2pc REQUIRED)
include_directories(${EMP-SH2PC_INCLUDE_DIRS})

ENABLE_TESTING()
ADD_SUBDIRECTORY(src)