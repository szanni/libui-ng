add_rules("mode.debug", "mode.release")

set_languages("c99", "cxx11")

set_warnings("everything")
add_cflags("-pedantic")

if is_os("linux") then
	add_requires("gtk3 >= 3.10")
end

if is_os("windows") then
--	add_rules("win.sdk.application")
--	add_syslinks("user32", "kernel32", "msimg32", "comdlg32")
	add_syslinks("comctl32", "gdi32", "uxtheme", "d2d1", "dwrite", "uuid", "ole32")
--	add_syslinks("oleaut32", "oleacc", "windowscodecs")
	if is_plat("mingw") and is_kind("static") then
		add_ldflags("-static", "-static-libgcc", "-static-libstdc++")
	end
end

target("ui-ng")
	set_kind("$(kind)") -- default: static
	add_files("common/*.c")
	if is_os("linux") then
		add_files("unix/*.c")
		add_packages("gtk3")
	end
	if is_os("windows") then
		add_files("windows/*.cpp")
	end

target("tester")
	set_kind("binary")
	add_deps("ui-ng")
	add_files("test/*.c")

