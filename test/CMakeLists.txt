add_executable(tester
	drawtests.c
	images.c
	main.c
	menus.c
	page1.c
	page2.c
	page3.c
	page4.c
	page5.c
	page6.c
	page7.c
	page7a.c
	page7b.c
	page7c.c
#	page8.c
#	page9.c
#	page10.c
	page11.c
	page12.c
	page13.c
	page14.c
	page15.c
	page16.c
	page17.c
	spaced.c
	resources.rc)
target_link_libraries(tester ${TargetLinkLibraries})

add_subdirectory(qa)

