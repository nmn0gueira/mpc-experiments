cmake_minimum_required(VERSION 3.14)
project (emptest)
set(NAME "emp-test")

find_package(OpenSSL REQUIRED)

set(L_FLAGS ${OPENSSL_LIBRARIES} "emp-tool")    # This is equivalent to EMP-TOOL_LIBRARIES in emp-tool-config.cmake

add_subdirectory(src)