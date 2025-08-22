SetCppProjectDefaults("Vibeout")
kind "WindowedApp"
debugdir(rootDir)

links { "Shaders" }
dependson { "Shaders" }

-- SDL
includedirs { path.join(externDir, "SDL3/include") }
links { "SDL" }
dependson { "SDL" }
--postbuildcommands { "{COPY} \"../libs/SDL3.dll\" \"%{cfg.targetdir}\"", "{ECHO} 'Copied SDL3.dll to %{cfg.targetdir}'" }

-- Vulkan
includedirs "%VULKAN_SDK%/Include"
libdirs "%VULKAN_SDK%/Lib"
links "vulkan-1"

-- VulkanMemoryAllocator (VMA)
includedirs { path.join(externDir, "VulkanMemoryAllocator/include") }

-- STB
includedirs { path.join(externDir, "stb") }

-- JoltPhysics
includedirs { path.join(externDir, "JoltPhysics")}
links { "Jolt" }
filter { "configurations:Debug" }
libdirs { path.join(externDir, "JoltPhysics/Build/VS2022_CL_Double/Debug") }
filter { "configurations:Release" }
libdirs { path.join(externDir, "JoltPhysics/Build/VS2022_CL_Double/Release") }
filter {}
