add_executable(qa
	qa.c
	main.c
	button.c
	checkbox.c
	entry.c
	label.c
	window.c
	resources.rc
)
target_link_libraries(qa ${TargetLinkLibraries})

