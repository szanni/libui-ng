macro(example name)
	add_executable(${name} ${name}/main.c resources.rc)
	target_link_libraries(${name} ${TargetLinkLibraries})
endmacro()

macro(examplecxx name)
	add_executable(${name} ${name}/main.cpp resources.rc)
	target_link_libraries(${name} ${TargetLinkLibraries})
endmacro()

example(controlgallery)
example(histogram)
examplecxx(cpp-multithread)
example(drawtext)
example(timer)
example(datetime)
example(hello-world)
example(window)

