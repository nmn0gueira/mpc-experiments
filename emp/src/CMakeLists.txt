include_directories(SYSTEM "emp/emp-tool" "emp/emp-ot" "emp/emp-sh2pc")

# These are used to compile the example programs with the same flags as emp-tool (removed from their CMakeLists.txt to avoid redundancy)
set(CMAKE_FOLDER ./emp/emp-tool)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

add_subdirectory("emp/emp-tool")
#add_subdirectory("emp/emp-ot")
#add_subdirectory("emp/emp-sh2pc")

# No compile options are set here, as they are set in emp-tool's emp-base.cmake
macro (add_example_executable _name)
	add_executable(${_name} "examples/${_name}.cpp")
	target_link_libraries(${_name} ${L_FLAGS})
endmacro()

add_example_executable(millionaire)
add_example_executable(xtabs)
add_example_executable(linreg)
add_example_executable(hist2d)