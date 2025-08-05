SetCppProjectDefaults("Vibeout")
kind "WindowedApp"

-- SDL
includedirs { path.join(externDir, "SDL3/include") }
links { "SDL" }

-- Vulkan
includedirs "%VULKAN_SDK%/Include"
libdirs "%VULKAN_SDK%/Lib"
links "vulkan-1"

-- VulkanMemoryAllocator (VMA)
includedirs { path.join(externDir, "VulkanMemoryAllocator/include") }
