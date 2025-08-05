SetCppProjectDefaults("Vibeout")
kind "WindowedApp"

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
