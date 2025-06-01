macro (add_example_executable _name)
	add_executable(${_name} "${_name}.cpp")
	#target_compile_options(${_name} PRIVATE ${C_OPT_FLAGS})	# Removed due to slight performance downgrade
	target_link_libraries(${_name} ${EMP-SH2PC_LIBRARIES})
endmacro()

add_example_executable(millionaire)
add_example_executable(hist2d)
add_example_executable(xtabs)
add_example_executable(linreg)