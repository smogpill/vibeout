// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Vibeout/Render/Renderer.h"
#include "Vibeout/Render/Shared/Shaders.h"
#include "Vibeout/Render/Shared/Textures.h"
#include "Vibeout/Render/Shared/Buffers.h"
#include "Vibeout/Render/Draw/PathTracer.h"

const uint32 vulkanAPIversion = VK_API_VERSION_1_3;

struct PickedSurfaceFormat
{
    VkSurfaceFormatKHR _surfaceFormat = {};
    VkFormat _swapchainViewFormat = VK_FORMAT_UNDEFINED;
};

static bool PickSurfaceFormatHDR(PickedSurfaceFormat* pickedFormat, const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats)
{
    VkSurfaceFormatKHR acceptableFormats[] =
    {
        { VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT }
    };

    for (const VkSurfaceFormatKHR& acceptable : acceptableFormats)
    {
        for (const VkSurfaceFormatKHR& available : availableSurfaceFormats)
        {
            if (acceptable.format == available.format 
                && acceptable.colorSpace == available.colorSpace)
            {
                pickedFormat->_surfaceFormat = available;
                pickedFormat->_swapchainViewFormat = acceptable.format;
                return true;
            }
        }
    }
    return false;
}

static bool PickSurfaceFormatSDR(PickedSurfaceFormat* pickedFormat, const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats)
{
    struct AcceptableFormat
    {
        VkFormat _format;
        VkFormat _swapchainViewFormat;
    };
    AcceptableFormat acceptableFormats[] =
    {
        {VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB},
        {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB},
        {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB},
        {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SRGB},
    };

    for (const auto& acceptable : acceptableFormats)
    {
        for (const VkSurfaceFormatKHR& available : availableSurfaceFormats)
        {
            if (acceptable._format == available.format)
            {
                pickedFormat->_surfaceFormat = available;
                pickedFormat->_swapchainViewFormat = acceptable._swapchainViewFormat;
                return true;
            }
        }
    }
    return false;
}

VkImageMemoryBarrier ImageBarrier()
{
    return VkImageMemoryBarrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED
    };
}

void QueueImageBarrier(const VkCommandBuffer& cmds, const VkImageMemoryBarrier& barrier)
{
    vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

Renderer::Renderer(SDL_Window& window, bool& result)
    : _window(window)
{
	result = Init();
}

Renderer::~Renderer()
{
    ShutdownPipelines();
    delete _pathTracer;
    delete _buffers;
    delete _textures;
    delete _shaders;

    // TODO missing destroys

    for (VkFence& fence : _fencesFrameSync)
        vkDestroyFence(_device, fence, nullptr);
    ShutdownSwapChain();
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
    VO_TRY(InitSwapChain());
    UpdateScreenImagesSize();
    bool result;
    _shaders = new Shaders(*this, result);
    VO_TRY(result);
    _textures = new Textures(*this, result);
    VO_TRY(result);
    _buffers = new Buffers(*this, result);
    VO_TRY(result);
    _pathTracer = new PathTracer(*this, result);
    VO_TRY(result);
    VO_TRY(InitPipelines());
	return true;
}

void Renderer::Render()
{
    if (BeginRender())
    {
        EvaluateAASettings();
        EndRender();
    }
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

    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_memProperties);

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

bool Renderer::InitSwapChain()
{
    _nbAccumulatedFrames = 0;

    VkSurfaceCapabilitiesKHR surfaceCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCaps);

    if (surfaceCaps.currentExtent.width == 0 || surfaceCaps.currentExtent.height == 0)
        return true;

    std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;
    {
        uint32 nbFormats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &nbFormats, nullptr);
        availableSurfaceFormats.resize(nbFormats, {});
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &nbFormats, availableSurfaceFormats.data());
    }

    PickedSurfaceFormat pickedFormat;
    bool surfaceFormatFound = false;
    if (_surfaceIsHDR != 0)
    {
        surfaceFormatFound = PickSurfaceFormatHDR(&pickedFormat, availableSurfaceFormats);
        _surfaceIsHDR = surfaceFormatFound;
    }
    else
    {
        _surfaceIsHDR = false;
    }
    if (!surfaceFormatFound)
        surfaceFormatFound = PickSurfaceFormatSDR(&pickedFormat, availableSurfaceFormats);
    VO_TRY(surfaceFormatFound, "No acceptable Vulkan surface format available!");

    _surfaceFormat = pickedFormat._surfaceFormat;

    std::vector<VkPresentModeKHR> availablePresentModes;
    {
        uint32 nbPresentModes = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &nbPresentModes, nullptr);
        availablePresentModes.resize(nbPresentModes, {});
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &nbPresentModes, availablePresentModes.data());
    }
   
    bool immediateModeAvailable = false;

    for (VkPresentModeKHR presentMode : availablePresentModes)
    {
        if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            immediateModeAvailable = true;
            break;
        }
    }

    if (_surfaceIsVSYNC)
    {
        _presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else if (immediateModeAvailable)
    {
        _presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else
    {
        _presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }

    if (surfaceCaps.currentExtent.width != ~0u)
    {
        _extentUnscaled = surfaceCaps.currentExtent;
    }
    else
    {
        auto [clientWidth, clientHeight] = GetSize();
        _extentUnscaled.width = std::min<uint>(surfaceCaps.maxImageExtent.width, clientWidth);
        _extentUnscaled.height = std::min<uint>(surfaceCaps.maxImageExtent.height, clientHeight);

        _extentUnscaled.width = std::max<uint>(surfaceCaps.minImageExtent.width, _extentUnscaled.width);
        _extentUnscaled.height = std::max<uint>(surfaceCaps.minImageExtent.height, _extentUnscaled.height);
    }

    {
        uint32 nbImages = std::max(surfaceCaps.minImageCount, 2u);
        if (surfaceCaps.maxImageCount > 0)
            nbImages = std::min(nbImages, surfaceCaps.maxImageCount);

        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = _surface;
        info.minImageCount = nbImages;
        info.imageFormat = _surfaceFormat.format;
        info.imageColorSpace = _surfaceFormat.colorSpace;
        info.imageExtent = _extentUnscaled;
        info.imageArrayLayers = 1; // only needs to be changed for stereoscopic rendering
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;
        info.preTransform = surfaceCaps.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = _presentMode;
        info.clipped = VK_FALSE;
        info.oldSwapchain = VK_NULL_HANDLE;

        VO_TRY_VK(vkCreateSwapchainKHR(_device, &info, nullptr, &_swapchain));
    }
   
    // Swap chain images
    {
        uint32 nbSwapChainImages = 0;
        VO_TRY_VK(vkGetSwapchainImagesKHR(_device, _swapchain, &nbSwapChainImages, nullptr));
        VO_TRY(nbSwapChainImages);
        _swapChainImages.resize(nbSwapChainImages, nullptr);
        VO_TRY_VK(vkGetSwapchainImagesKHR(_device, _swapchain, &nbSwapChainImages, _swapChainImages.data()));
    }
   
    // Swap chain image views
    {
        _swapChainImageViews.resize(_swapChainImages.size(), nullptr);
        for (uint i = 0; i < _swapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = _swapChainImages[i];
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = pickedFormat._swapchainViewFormat;
#if 1
            info.components =
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            };
#endif
            info.subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };

            VO_TRY_VK(vkCreateImageView(_device, &info, nullptr, &_swapChainImageViews[i]));
        }
    }

    {
        VkCommandBuffer cmds = BeginCommandBuffer(_graphicsCommandBuffers);

        for (VkImage& image : _swapChainImages)
        {
            VkImageMemoryBarrier barrier = ImageBarrier();
            barrier.image = image;
            barrier.subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            QueueImageBarrier(cmds, barrier);
        }

        VO_TRY(SubmitCommandBufferSimple(cmds, _graphicsQueue));
        WaitIdle(_graphicsQueue, _graphicsCommandBuffers);
    }

    return true;
}

void Renderer::ShutdownSwapChain()
{
    for (VkImageView& imageView : _swapChainImageViews)
    {
        vkDestroyImageView(_device, imageView, nullptr);
    }
    _swapChainImageViews.clear();
    _swapChainImages.clear();

    if (_swapchain)
    {
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        _swapchain = nullptr;
    }
}

bool Renderer::InitPipelines()
{
    VO_TRY(_pathTracer->InitPipelines());
    return true;
}

void Renderer::ShutdownPipelines()
{
    _pathTracer->ShutdownPipelines();
}

bool Renderer::Recreate()
{
    vkDeviceWaitIdle(_device);
    ShutdownPipelines();
    _textures->ShutImages();
    ShutdownSwapChain();
    UpdateScreenImagesSize();
    VO_TRY(InitSwapChain());
    VO_TRY(_textures->InitImages());
    VO_TRY(InitPipelines());
    _waitForIdleFrames = maxFramesInFlight * 2;
    return true;
}

void Renderer::UpdateScreenImagesSize()
{
    _extentScreenImages = GetScreenImageExtent();
    _extentTAAImages.width = std::max(_extentScreenImages.width, _extentUnscaled.width);
    _extentTAAImages.height = std::max(_extentScreenImages.height, _extentUnscaled.height);
}

void Renderer::EvaluateAASettings()
{
    _antiAliasing = false;
    _extentTAAOutput = _extentRender;

    /*
    if (!_useDenoising)
        return;

   
    int flt_taa = _desiredTAA;

    if (flt_taa == AA_MODE_TAA)
    {
        _effectiveAAMode = AA_MODE_TAA;
    }
    else if (flt_taa == AA_MODE_UPSCALE) // TAAU or TAA+FSR
    {
        if (_extent_render.width > _extent_unscaled.width || _extent_render.height > _extent_unscaled.height)
        {
            _effectiveAAMode = AA_MODE_TAA;
        }
        else
        {
            _effectiveAAMode = AA_MODE_UPSCALE;
            _extent_taa_output = _extent_unscaled;
        }
    }
    */
}

bool Renderer::BeginRender()
{
    VO_TRY(_physicalDevice);
    VO_TRY(_device);
    VO_TRY(_surface);

    _currentFrameInFlight = _frameCounter % maxFramesInFlight;
    VkResult fenceResult = vkWaitForFences(_device, 1, &_fencesFrameSync[_currentFrameInFlight], VK_TRUE, ~((uint64_t)0));
    if (fenceResult == VK_ERROR_DEVICE_LOST)
    {
        VO_ERROR("Device lost!");
        return false;
    }

    //vkResetFences(device, 1, &_fencesFrameSync[_currentFrameInFlight]);

    if (!_swapchain)
    {
        VkSurfaceCapabilitiesKHR surfaceCaps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCaps);

        // Un-minimized again?
        if (surfaceCaps.currentExtent.width != 0 && surfaceCaps.currentExtent.height != 0)
        {
            VO_TRY(Recreate());
        }
    }

    _extentRender = GetRenderExtent();

    VkExtent2D extentScreenImages = GetScreenImageExtent();

    if (extentScreenImages.width != _extentScreenImages.width
        || extentScreenImages.height != _extentScreenImages.height
        || _wantHDR != _surfaceIsHDR
        || _wantVSYNC != _surfaceIsVSYNC)
    {
        _extentScreenImages = extentScreenImages;
        VO_TRY(Recreate());
    }

    bool retry = false;
    do
    {
        if (!_swapchain)
            return false;

        VkResult swapchainResult = vkAcquireNextImageKHR(_device, _swapchain, ~((uint64_t)0),
            _semaphoreGroups[_currentFrameInFlight]._imageAvailable, VK_NULL_HANDLE, &_currentSwapChainImageIdx);
        if (swapchainResult == VK_ERROR_OUT_OF_DATE_KHR || swapchainResult == VK_SUBOPTIMAL_KHR)
        {
            VO_CHECK(Recreate());
            retry = true;
        }
        else if (swapchainResult != VK_SUCCESS)
        {
            VO_ERROR("Error {} in vkAcquireNextImageKHR\n", (int)swapchainResult);
        }
    } while (retry);

    if (_waitForIdleFrames)
    {
        vkDeviceWaitIdle(_device);
        --_waitForIdleFrames;
    }

    vkResetFences(_device, 1, &_fencesFrameSync[_currentFrameInFlight]);

    ResetCommandBuffers(_graphicsCommandBuffers);

    //VO_TRY(_draw->ClearStretchPics());
    return true;
}

void Renderer::EndRender()
{
    if (!_swapchain)
    {
        //coCHECK(_draw->ClearStretchPics());
        return;
    }

    VkCommandBuffer commandBuffer = BeginCommandBuffer(_graphicsCommandBuffers);
    if (_frameReady)
    {
        //coCHECK(_draw->FinalBlit(commandBuffer, ImageID::TAA_OUTPUT, _extentTAAOutput));
        _frameReady = false;
    }

    //coCHECK(_draw->SubmitStretchPics(commandBuffer));

    VkSemaphore waitSemaphores = _semaphoreGroups[_currentFrameInFlight]._imageAvailable;
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSemaphore signalSemaphore = _semaphoreGroups[_currentFrameInFlight]._renderFinished;

    assert(_graphicsQueue);
    VO_CHECK(SubmitCommandBuffer(commandBuffer, _graphicsQueue, 1,
        &waitSemaphores, &waitStages, 1, &signalSemaphore, _fencesFrameSync[_currentFrameInFlight]));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &signalSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.pImageIndices = &_currentSwapChainImageIdx;

    VkResult presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
        VO_CHECK(Recreate());

    ++_frameCounter;
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

std::pair<uint32, uint32> Renderer::GetSize() const
{
    int width = 0;
    int height = 0;
    if ((SDL_GetWindowFlags(&_window) & SDL_WINDOW_MINIMIZED) == 0)
    {
        SDL_GetWindowSize(&_window, &width, &height);
    }
    return std::pair<uint32, uint32>(width, height);
}

bool Renderer::GetMemoryType(uint32 memReqTypeBits, VkMemoryPropertyFlags memProps, uint32& outMemType) const
{
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
    {
        if (memReqTypeBits & (1 << i))
        {
            if ((_memProperties.memoryTypes[i].propertyFlags & memProps) == memProps)
            {
                outMemType = i;
                return true;
            }
        }
    }

    VO_ERROR("Failed to find memory type with provided flags: {}", memProps);
    return false;
}

VkDeviceSize Renderer::GetAvailableVideoMemory() const
{
    VkDeviceSize mem = 0;
    for (uint32_t i = 0; i < _memProperties.memoryHeapCount; ++i)
    {
        if ((_memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
            mem += _memProperties.memoryHeaps[i].size;
    }
    return mem;
}

bool Renderer::AllocateGPUMemory(VkMemoryRequirements memReq, VkDeviceMemory* memory)
{
    uint32 memoryType;
    VO_TRY(GetMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryType));
    VkMemoryAllocateInfo info =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReq.size,
        .memoryTypeIndex = memoryType
    };

    VO_TRY_VK(vkAllocateMemory(_device, &info, nullptr, memory));
    return true;
}

VkExtent2D Renderer::GetScreenImageExtent() const
{
    VkExtent2D result;
    /*
    if (cvar_drs_enable->integer)
    {
        int image_scale = max(cvar_drs_minscale->integer, cvar_drs_maxscale->integer);

        // In case FSR enable we'll always upscale to 100% and thus need at least the unscaled extent
        if (vkpt_fsr_is_enabled())
            image_scale = max(image_scale, 100);

        result.width = (uint32_t)(qvk.extent_unscaled.width * (float)image_scale / 100.f);
        result.height = (uint32_t)(qvk.extent_unscaled.height * (float)image_scale / 100.f);
    }
    else*/
    {
        result.width = std::max(_extentRender.width, _extentUnscaled.width);
        result.height = std::max(_extentRender.height, _extentUnscaled.height);
    }

    result.width = (result.width + 1) & ~1;

    return result;
}

VkExtent2D Renderer::GetRenderExtent() const
{
    float scale = 1.0f;
    /*
    if (_drs_effective_scale)
    {
        scale = _drs_effective_scale;
    }
    else
    {
        scale = scr_viewsize->integer;
        if (cvar_drs_enable->integer)
        {
            // Ensure render extent stays below get_screen_image_extent() result
            scale = coMin(cvar_drs_maxscale->integer, scale);
        }
    }
    */

    VkExtent2D result;
    result.width = (uint32_t)(_extentUnscaled.width * scale);
    result.height = (uint32_t)(_extentUnscaled.height * scale);

    result.width = (result.width + 1) & ~1;

    return result;
}

void Renderer::WaitIdle(VkQueue queue, CommandBufferGroup& group)
{
    vkQueueWaitIdle(queue);
    ResetCommandBuffers(group);
}

void Renderer::ResetCommandBuffers(CommandBufferGroup& group)
{
    group._usedThisFrame = 0;
}

VkCommandBuffer Renderer::BeginCommandBuffer(CommandBufferGroup& group)
{
    if (group._usedThisFrame == group._maxNbPerFrame)
    {
        const uint32 newMax = std::max(4u, group._maxNbPerFrame * 2u);
        const uint32 nbNew = newMax - group._maxNbPerFrame;

        VkCommandBufferAllocateInfo allocInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = group._commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = nbNew
        };

        for (std::vector<VkCommandBuffer>& commandBuffers : group._commandBuffers)
        {
            commandBuffers.resize(newMax, nullptr);
            VO_CHECK_VK(vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffers[group._maxNbPerFrame]));
        }

        group._maxNbPerFrame = newMax;
    }

    VkCommandBuffer commandBuffer = group._commandBuffers[_currentFrameInFlight][group._usedThisFrame];

    VkCommandBufferBeginInfo beginInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };
    VO_CHECK_VK(vkResetCommandBuffer(commandBuffer, 0));
    VO_CHECK_VK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    ++group._usedThisFrame;
    return commandBuffer;
}

bool Renderer::SubmitCommandBuffer(VkCommandBuffer cmds, VkQueue queue, int wait_semaphore_count,
    VkSemaphore* wait_semaphores, VkPipelineStageFlags* wait_stages, int signal_semaphore_count,
    VkSemaphore* signal_semaphores, VkFence fence)
{
    VO_TRY_VK(vkEndCommandBuffer(cmds));

    VkSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = wait_semaphore_count;
    info.pWaitSemaphores = wait_semaphores;
    info.pWaitDstStageMask = wait_stages;
    info.signalSemaphoreCount = signal_semaphore_count;
    info.pSignalSemaphores = signal_semaphores;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &cmds;

    VO_TRY_VK(vkQueueSubmit(queue, 1, &info, fence));
    return true;
}

bool Renderer::SubmitCommandBufferSimple(VkCommandBuffer cmds, VkQueue queue)
{
    return SubmitCommandBuffer(cmds, queue, 0, nullptr, nullptr, 0, nullptr, nullptr);
}
