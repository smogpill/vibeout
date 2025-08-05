rootDir = path.getabsolute("..")
externDir = path.join(rootDir, "Extern")
sourceDir = path.join(rootDir, "Source")

function SetProjectDefaults(name)
	local buildDir = "../../Build"
	project(name)
	location(path.join(buildDir, "Projects"))
	architecture "x64"
	kind "StaticLib"
	objdir(path.join(buildDir, "Obj"))
	targetdir(path.join(buildDir, "Bin/$(Configuration)"))
	libdirs { "$(OutDir)" }
	includedirs("..")

	debugdir "$(OutDir)"
	configurations { "Debug", "Release" }

	filter{"configurations:Debug"}
		defines {"VO_DEBUG"}
	filter { "configurations:Release" }
		optimize "On"
	filter{"configurations:Release"}
		defines {"VO_RELEASE"}
	filter {}	
end

function SetCppProjectDefaults(name)
	SetProjectDefaults(name)
	rtti "Off"
	language "C++"
	exceptionhandling "Off"
	vectorextensions "SSE2"
	symbols "On"
	cppdialect "C++20"
	runtime "Release" -- Even on debug builds, Unreal is setup this way anyway. But can't use the CRT library memory leaks detector
	fatalwarnings { "All"}
	flags { "MultiProcessorCompile" }
	files { "**.cpp", "**.h", "**.hpp", "**.inl", "**.natvis" }

	if os.isfile("PCH.h") then
		pchheader("PCH.h")
		pchsource('PCH.cpp')
	end

	filter { "gmake" }
		buildoptions { "-Wno-reorder", "-Wno-deprecated" }
		includedirs { gmakeIncludeDir }

	filter { "action:vs*" }
		files { "*.natvis"}
		defines { "_HAS_EXCEPTIONS=0" }
		buildoptions {"/Zc:preprocessor"} -- Required for C++20's __VA_OPT__() for some reason
		--flags { "StaticRuntime" }
		--linkoptions { "/ENTRY:mainCRTStartup" } -- TODO: Not working with DLL, should avoid that case somehow.
		linkoptions {"/ignore:4221"} -- warns when .cpp are empty depending on the order of obj linking.
		
	filter { "configurations:Release" }
		optimize "On"
		omitframepointer "On"
	filter {}
end

workspace "Vibeout"
	configurations { "Debug", "Release" }
	location "Workspaces"
	
	include "../Source/Vibeout"
	include "../Source/Shaders"

	externalproject "SDL"
   	location "../Extern/SDL3/VisualC/SDL"
   	uuid "81CE8DAF-EBB2-4761-8E45-B71ABCCA8C68"
   	kind "SharedLib"
   	language "C++"
   	architecture "x64"
