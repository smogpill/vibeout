// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Renderer.h"

const uint32 vulkanAPIversion = VK_API_VERSION_1_2;

Renderer::Renderer(SDL_Window& window, bool& result)
    : _window(window)
{
	result = Init();
}

Renderer::~Renderer()
{
    if (_vmaAllocator)
        vmaDestroyAllocator(_vmaAllocator);
    if (_device)
        vkDestroyDevice(_device, nullptr);
    if (_surface)
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
    if (_instance)
        vkDestroyInstance(_instance, nullptr);
}

bool Renderer::Init()
{
    VO_TRY(InitInstance());
    VO_TRY(InitSurface());
    VO_TRY(InitPhysicalDevice());
    VO_TRY(InitQueueFamilies());
    VO_TRY(InitLogicalDevice());
    VO_TRY(InitVMA());
    VO_TRY(InitCommandPools());
    VO_TRY(InitSemaphores());
    VO_TRY(InitFences());
	return true;
}

bool Renderer::InitInstance()
{
    VkApplicationInfo appInfo = {};
    {
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vibeout";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = vulkanAPIversion;
    };

    std::vector<const char*> chosenExtensions;

    // Get required Vulkan extensions from SDL
    uint nbExtensionsForSDL = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&nbExtensionsForSDL);
    if (!nbExtensionsForSDL)
    {
        printf("Failed to get Vulkan extension count: %s\n", SDL_GetError());
        return false;
    }
   
    for (uint i = 0; i < nbExtensionsForSDL; ++i)
    {
        chosenExtensions.push_back(extensions[i]);
    }

#ifdef coDEV
    chosenExtensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    // Create Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    {
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 0;
        createInfo.enabledExtensionCount = (uint32)chosenExtensions.size();
        createInfo.ppEnabledExtensionNames = chosenExtensions.data();
#ifdef VO_DEBUG
        const char* validationLayers[] =
        {
            "VK_LAYER_KHRONOS_validation"
        };
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.enabledLayerCount = std::size(validationLayers);
#endif
    };

    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
    {
        printf("Failed to create Vulkan instance\n");
        return false;
    }

#ifdef VO_DEBUG
    // Debug utils
    {
        /*
        auto vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
        if (vkCreateDebugUtilsMessenger)
            vkCreateDebugUtilsMessenger(_instance, &debugInfo, nullptr, &_debugUtilsMessenger);
        */

        _vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(_instance, "vkCmdBeginDebugUtilsLabelEXT");
        _vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(_instance, "vkCmdEndDebugUtilsLabelEXT");
        _vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(_instance, "vkCmdInsertDebugUtilsLabelEXT");
        _vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(_instance, "vkQueueBeginDebugUtilsLabelEXT");
        _vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(_instance, "vkQueueEndDebugUtilsLabelEXT");
        _vkQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(_instance, "vkQueueInsertDebugUtilsLabelEXT");
        _vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(_instance, "vkSetDebugUtilsObjectNameEXT");
        _vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(_instance, "vkSetDebugUtilsObjectTagEXT");
    }
#endif

    return true;
}

bool Renderer::InitSurface()
{
    if (!SDL_Vulkan_CreateSurface(&_window, _instance, nullptr, &_surface))
    {
        printf("Failed to create Vulkan surface: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool Renderer::InitPhysicalDevice()
{
    uint32 nbDevices = 0;
    vkEnumeratePhysicalDevices(_instance, &nbDevices, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(nbDevices);
    vkEnumeratePhysicalDevices(_instance, &nbDevices, physicalDevices.data());

    // Discrete GPU first
    auto compare = [&](const VkPhysicalDevice& a, const VkPhysicalDevice& b)
        {
            VkPhysicalDeviceProperties propsA;
            VkPhysicalDeviceProperties propsB;
            vkGetPhysicalDeviceProperties(a, &propsA);
            vkGetPhysicalDeviceProperties(b, &propsB);
            if (propsA.deviceType != propsB.deviceType)
                return propsA.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            return true;
        };
    std::sort(physicalDevices.begin(), physicalDevices.end(), compare);

    for (const auto& device : physicalDevices)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        uint32 nbQueueFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &nbQueueFamilies, nullptr);

        if (nbQueueFamilies > 0)
        {
            _physicalDevice = device;
            std::cout << "Selected GPU: " << props.deviceName << "\n";
            break;
        }
    }

    if (!_physicalDevice)
    {
        std::cerr << "Failed to find suitable GPU!\n";
        return false;
    }

    return true;
}

bool Renderer::InitQueueFamilies()
{
    uint32 nbQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &nbQueueFamilies, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(nbQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &nbQueueFamilies, queueFamilies.data());

    // Find the graphics queue family
    for (uint32_t i = 0; i < nbQueueFamilies; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            _graphicsQueueFamilyIndex = i;
            break;
        }
    }

    if (_graphicsQueueFamilyIndex == uint32(-1))
    {
        std::cerr << "Failed to find graphics queue family!\n";
        return false;
    }
    return true;
}

bool Renderer::InitLogicalDevice()
{
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;

    // Enable device extensions (swapchain is essential)
    const char* deviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    features.features.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
    features12.pNext = &features;

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = &features12;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = std::size(deviceExtensions);
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;

    VO_TRY_VK(vkCreateDevice(_physicalDevice, &deviceInfo, nullptr, &_device));

    vkGetDeviceQueue(_device, _graphicsQueueFamilyIndex, 0, &_graphicsQueue);

    return true;
}

bool Renderer::InitVMA()
{
    assert(_physicalDevice);
    assert(_device);
    assert(_instance);
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = vulkanAPIversion;
    allocatorCreateInfo.physicalDevice = _physicalDevice;
    allocatorCreateInfo.device = _device;
    allocatorCreateInfo.instance = _instance;

    assert(_vmaAllocator == nullptr);
    if (vmaCreateAllocator(&allocatorCreateInfo, &_vmaAllocator) != VK_SUCCESS)
    {
        std::cerr << "Failed to create the VMA allocator!\n";
        return false;
    }
    return true;
}

bool Renderer::InitCommandPools()
{
    VkCommandPoolCreateInfo info = {};
    {
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = _graphicsQueueFamilyIndex;
    }

    VO_TRY_VK(vkCreateCommandPool(_device, &info, nullptr, &_graphicsCommandBuffers._commandPool));
    return true;
}

bool Renderer::InitSemaphores()
{
    for (SemaphoreGroup& group : _semaphoreGroups)
    {
        VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VO_TRY_VK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &group._imageAvailable));
        VO_TRY_VK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &group._renderFinished));
        VO_TRY_VK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &group._transferFinished));
        VO_TRY_VK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &group._traceFinished));
        SetObjectName(group._imageAvailable, "ImageAvailable");
        SetObjectName(group._renderFinished, "RenderFinished");
        SetObjectName(group._transferFinished, "TransferFinished");
        SetObjectName(group._traceFinished, "TraceFinished");
    }

    return true;
}

bool Renderer::InitFences()
{
    VkFenceCreateInfo fence_info =
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT, /* fence's initial state set to be signaled
                                                    to make program not hang */
    };
    for (VkFence& fence : _fencesFrameSync)
    {
        VO_TRY_VK(vkCreateFence(_device, &fence_info, nullptr, &fence));
        SetObjectName(fence, "FrameSync");
    }
    return true;
}

void Renderer::SetObjectName(uint64 object, VkObjectType objectType, const char* name)
{
    if (_vkSetDebugUtilsObjectNameEXT && _device)
    {
        VkDebugUtilsObjectNameInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
        info.objectType = objectType;
        info.objectHandle = object;
        info.pObjectName = name;
        _vkSetDebugUtilsObjectNameEXT(_device, &info);
    }
}

void Renderer::BeginCommandsLabel(VkCommandBuffer commands, const char* name)
{
    if (_vkCmdBeginDebugUtilsLabelEXT && commands)
    {
        VkDebugUtilsLabelEXT label{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        label.pLabelName = name;
        _vkCmdBeginDebugUtilsLabelEXT(commands, &label);
    }
}

void Renderer::EndCommandsLabel(VkCommandBuffer commands)
{
    if (_vkCmdEndDebugUtilsLabelEXT && commands)
    {
        _vkCmdEndDebugUtilsLabelEXT(commands);
    }
}

void Renderer::InsertCommandsLabel(VkCommandBuffer commands, const char* name)
{
    if (_vkCmdInsertDebugUtilsLabelEXT && commands)
    {
        VkDebugUtilsLabelEXT label{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        label.pLabelName = name;
        _vkCmdInsertDebugUtilsLabelEXT(commands, &label);
    }
}

void Renderer::BeginQueueLabel(VkQueue queue, const char* name)
{
    if (_vkQueueBeginDebugUtilsLabelEXT && queue)
    {
        VkDebugUtilsLabelEXT label{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        label.pLabelName = name;
        _vkQueueBeginDebugUtilsLabelEXT(queue, &label);
    }
}

void Renderer::EndQueueLabel(VkQueue queue)
{
    if (_vkQueueEndDebugUtilsLabelEXT && queue)
    {
        _vkQueueEndDebugUtilsLabelEXT(queue);
    }
}

void Renderer::InsertQueueLabel(VkQueue queue, const char* name)
{
    if (_vkQueueInsertDebugUtilsLabelEXT && queue)
    {
        VkDebugUtilsLabelEXT label{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
        label.pLabelName = name;
        _vkQueueInsertDebugUtilsLabelEXT(queue, &label);
    }
}
