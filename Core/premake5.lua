project "Core"
	kind "StaticLib"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"Source/**.h",
		"Source/**.c",
		"Source/**.hpp",
		"Source/**.cpp",

        "vendor/GLAD/src/glad.c",

		"vendor/stb/**.h",
		"vendor/stb/**.cpp",

		"Platform/" .. firstToUpper(os.target()) .. "/**.hpp",
		"Platform/" .. firstToUpper(os.target()) .. "/**.cpp", 
	}

    links {
        "GLAD",
        "GLFW"
    }

	includedirs { 
        "Source/", 
        "vendor/", 
        "vendor/GLAD/include",
        "vendor/GLFW/include"
    }

	IncludeDependencies()

	defines { 
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
    }
    flags { "NoPCH" }

	filter "system:windows"
        systemversion "latest"
        defines { "WIN32_LEAN_AND_MEAN", "NOMINMAX", "GLFW_INCLUDE_NONE" }
        links { "opengl32" }

	filter "system:linux"
		defines { "__EMULATE_UUID", "BACKWARD_HAS_DW", "BACKWARD_HAS_LIBUNWIND" }
		links { "dw", "dl", "unwind", "pthread" }

	filter "configurations:Debug or configurations:Debug-AS"
		symbols "On"
		defines { "_DEBUG", "ACL_ON_ASSERT_ABORT", }

	filter { "system:windows", "configurations:Debug-AS" }	
		sanitize { "Address" }
		flags { "NoRuntimeChecks", "NoIncrementalLink" }

	filter "configurations:Release"
		optimize "On"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }
		defines { "NDEBUG", }

	filter "configurations:Dist"
		optimize "On"
		symbols "Off"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }
