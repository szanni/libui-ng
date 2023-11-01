cmake_minimum_required(VERSION 3.15)

# Need to be set before project(), see docs.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "Minimum macOS deployment version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build fat binary")

project(libui)

# Strict C99 and C++11
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)
if(WIN32)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED True)
	set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_POSITION_INDEPENDENT_CODE True)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-Wno-unused-parameter)
endif()

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build as shared library" ON)

# Defines: CommonSourceList
add_subdirectory(common)

if(UNIX AND NOT APPLE)
	add_subdirectory(unix) # Defines: TargetSourceList
	set(TargetHeader ui_unix.h)
	# GTK3
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.10)
	include_directories(${GTK3_INCLUDE_DIRS})
	link_directories(${GTK3_LIBRARY_DIRS})
	add_definitions(${GTK3_CFLAGS_OTHER})
	set(TargetLinkLibraries ${GTK3_LIBRARIES})
endif()
if(WIN32)
	add_subdirectory(windows) # Defines: TargetSourceList
	set(TargetHeader ui_windows.h)
	set(TargetLinkLibraries user32 comctl32 gdi32 uxtheme d2d1 dwrite ole32 uuid windowscodecs)
	if (MINGW)
		if(BUILD_SHARED_LIBS)
			message(FATAL_ERROR "Shared library builds are currently not possible with "
				"mingw, as isolation awarness seems to be lacking. See: "
				"https://sourceforge.net/p/mingw-w64/feature-requests/44/ "
				"https://docs.microsoft.com/en-us/windows/win32/controls/cookbook-overview "
				"https://docs.microsoft.com/en-us/windows/win32/sbscs/isolating-components")
		else()
			# Statically link stdlib to make executables truly static
			string(APPEND CMAKE_EXE_LINKER_FLAGS " -static -static-libgcc -static-libstdc++")
		endif()
	endif()
	if(MSVC)
		add_link_options(/LARGEADDRESSAWARE /INCREMENTAL:NO /MANIFEST:NO)
	endif()
endif()
if(APPLE)
	add_subdirectory(darwin) # Defines: TargetSourceList
	set(TargetHeader ui_darwin.h)
endif()

add_library(ui ${CommonSourceList} ${TargetSourceList})
target_link_libraries(ui ${TargetLinkLibraries})
list(APPEND TargetLinkLibraries ui)
target_sources(ui PUBLIC FILE_SET HEADERS BASE_DIRS . FILES ui.h ${TargetHeader})

# TODO rename variable to something like _UI_BUILD_LIBRARY
target_compile_definitions(ui PRIVATE libui_EXPORTS)
if(NOT BUILD_SHARED_LIBS)
	target_compile_definitions(ui PUBLIC _UI_STATIC)
endif()

# Install
install(TARGETS ui
	LIBRARY DESTINATION lib
	FILE_SET HEADERS DESTINATION include
)

add_subdirectory(examples)
add_subdirectory(test)

