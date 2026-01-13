project "App"
	kind "ConsoleApp"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	links { "Core" }

	defines { "GLM_FORCE_DEPTH_ZERO_TO_ONE", }

	files  { 
		"Source/**.h",
		"Source/**.c",
		"Source/**.hpp",
		"Source/**.cpp",
		
		-- Shaders
		"Resources/Shaders/**.glsl",
		"Resources/Shaders/**.glslh",
		"Resources/Shaders/**.hlsl",
		"Resources/Shaders/**.hlslh",
		"Resources/Shaders/**.slh",
	}

	includedirs  {
		"Source/",

		"../Core/Source/",
		"../Core/vendor/"
	}

	filter { "system:windows", "configurations:Debug or configurations:Debug-AS" }
		postbuildcommands {
			'{COPY} "../Core/vendor/assimp/bin/windows/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"',
		}

	filter { "system:windows", "configurations:Release or configurations:Dist" }
		postbuildcommands {
			'{COPY} "../Core/vendor/assimp/bin/windows/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
		}

	filter "system:windows" 
		systemversion "latest"
		defines { "GLFW_INCLUDE_NONE"}

	filter "system:linux"
		defines { "__EMULATE_UUID", "BACKWARD_HAS_DW", "BACKWARD_HAS_LIBUNWIND" }
		links { "dw", "dl", "unwind", "pthread" }

		result, err = os.outputof("pkg-config --libs gtk+-3.0")
		linkoptions { result }

	filter "configurations:Debug or configurations:Debug-AS"
		symbols "On"
		ProcessDependencies("Debug")

	filter { "system:windows", "configurations:Debug-AS" }
		sanitize { "Address" }
		flags { "NoRuntimeChecks", "NoIncrementalLink" }

	filter "configurations:Release"
		optimize "On"
        vectorextensions "AVX2"
        isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

		ProcessDependencies("Release")

    filter "configurations:Dist"
        optimize "On"
        vectorextensions "AVX2"
        isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

		ProcessDependencies("Dist ")