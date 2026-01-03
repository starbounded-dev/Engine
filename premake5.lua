include "./vendor/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Engine"
	configurations { "Debug", "Debug-AS", "Release", "Dist" }
	startproject "App"
    conformancemode "On"

	language "C++"
	cppdialect "C++23"
	staticruntime "Off"

	solution_items { ".editorconfig" }

	flags { "MultiProcessorCompile" }

	-- NOTE(Peter): Don't remove this. Please never use Annex K functions ("secure", e.g _s) functions.
	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"NOMINMAX",
		"SPDLOG_USE_STD_FORMAT",
		"_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING",
	}

    filter "action:vs*"
        linkoptions { "/ignore:4099" } -- NOTE(Peter): Disable no PDB found warning
        disablewarnings { "4068" } -- Disable "Unknown #pragma mark warning"

	filter "language:C++ or language:C"
		architecture "x86_64"

	filter "configurations:Debug or configurations:Debug-AS"
		optimize "Off"
		symbols "On"

	filter { "system:windows", "configurations:Debug-AS" }	
		sanitize { "Address" }
		flags { "NoRuntimeChecks", "NoIncrementalLink" }

	filter "configurations:Release"
		optimize "On"
		symbols "Default"
		defines { "NDEBUG" }

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"

	filter "system:windows"
		buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Core/vendor/GLFW"
group ""

group "Dependencies/Text"

group ""

group "Dependencies/Coral"

group ""

group "Dependencies/Renderer"
	include "Core/vendor/GLAD"
group ""

group "Dependencies/Physics"

group ""

group "Core"
	include "Core"
group ""

group "Tools"
group ""

group "Runtime"
	include "App"
group ""
