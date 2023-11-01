include(FetchContent)

FetchContent_Declare(
	cmocka
	GIT_REPOSITORY https://git.cryptomilk.org/projects/cmocka.git
	GIT_TAG        f5e2cd77c88d9f792562888d2b70c5a396bfbf7a # cmocka-1.1.5
	GIT_SHALLOW    1
)

set(WITH_STATIC_LIB ON CACHE BOOL "Static library")
set(UNIT_TESTING OFF CACHE BOOL "No unit testing")
set(WITH_EXAMPLES OFF CACHE BOOL "No examples")

FetchContent_MakeAvailable(cmocka)

add_executable(unit
	main.c
	init.c
	slider.c
	spinbox.c
	label.c
	button.c
	combobox.c
	checkbox.c
	radiobuttons.c
	entry.c
	menu.c
	progressbar.c
	resources.rc
)
target_link_libraries(unit ${TargetLinkLibraries} cmocka-static)
add_test(NAME "Unit Tests" COMMAND unit)

