workspace "SfxUnpak"
	configurations
	{
		"Debug",
		"Release",
	}

	location "build"

project "SfxUnpak"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetname "SfxUnpak"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".exe"
	
	files { "src/AudioContainers.cpp" }
	files { "src/Common.cpp" }
	files { "src/SfxUnpak.cpp" }
	files { "src/AudioContainers.h" }
	files { "src/Common.h" }
	files { "src/Wav.h" }

	characterset ("MBCS")
	toolset ("v141_xp")
	floatingpoint "Fast"
	buildoptions { "/Zc:sizedDealloc-" }
	links { "legacy_stdio_definitions" }
	linkoptions { "/SAFESEH:NO /IGNORE:4222" }
	defines { "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_USE_32BIT_TIME_T", "NOMINMAX" }

	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols "full"
		optimize "off"
		runtime "debug"
		editAndContinue "off"
		flags { "NoIncrementalLink" }
		staticruntime "on"

	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "on"
		optimize "speed"
		runtime "release"
		staticruntime "on"
		flags { "LinkTimeOptimization" }
		linkoptions { "/OPT:NOICF" }



project "SfxUnpak_PS2"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetname "SfxUnpak_PS2"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".exe"
	
	files { "src/AudioContainers.cpp" }
	files { "src/Common.cpp" }
	files { "src/SfxUnpak_PS2.cpp" }
	files { "src/AudioContainers.h" }
	files { "src/Common.h" }
	files { "src/Wav.h" }
	files { "src/Vag.h" }

	characterset ("MBCS")
	toolset ("v141_xp")
	floatingpoint "Fast"
	buildoptions { "/Zc:sizedDealloc-" }
	links { "legacy_stdio_definitions" }
	linkoptions { "/SAFESEH:NO /IGNORE:4222" }
	defines { "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_USE_32BIT_TIME_T", "NOMINMAX" }

	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols "full"
		optimize "off"
		runtime "debug"
		editAndContinue "off"
		flags { "NoIncrementalLink" }
		staticruntime "on"

	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "on"
		optimize "speed"
		runtime "release"
		staticruntime "on"
		flags { "LinkTimeOptimization" }
		linkoptions { "/OPT:NOICF" }
	

project "SfxUnpak_Xbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetname "SfxUnpak_Xbox"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".exe"
	
	files { "src/AudioContainers.cpp" }
	files { "src/Common.cpp" }
	files { "src/ImaADPCM.cpp" }
	files { "src/SfxUnpak_Xbox.cpp" }
	files { "src/AudioContainers.h" }
	files { "src/Common.h" }
	files { "src/ImaADPCM.h" }
	files { "src/Wav.h" }

	characterset ("MBCS")
	toolset ("v141_xp")
	floatingpoint "Fast"
	buildoptions { "/Zc:sizedDealloc-" }
	links { "legacy_stdio_definitions" }
	linkoptions { "/SAFESEH:NO /IGNORE:4222" }
	defines { "XBOX", "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_USE_32BIT_TIME_T", "NOMINMAX" }

	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols "full"
		optimize "off"
		runtime "debug"
		editAndContinue "off"
		flags { "NoIncrementalLink" }
		staticruntime "on"

	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "on"
		optimize "speed"
		runtime "release"
		staticruntime "on"
		flags { "LinkTimeOptimization" }
		linkoptions { "/OPT:NOICF" }
