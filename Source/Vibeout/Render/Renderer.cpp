// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Renderer.h"

Renderer::Renderer(SDL_Window& window, bool& result)
    : _window(window)
{
	result = Init();
}

Renderer::~Renderer()
{
    if (_surface)
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
    if (_instance)
        vkDestroyInstance(_instance, nullptr);
}

bool Renderer::Init()
{
    if (!InitInstance())
        return false;
    if (!InitSurface())
        return false;
    if (!InitPhysicalDevice())
        return false;
    if (!InitQueueFamilies())
        return false;
    if (!InitLogicalDevice())
        return false;

	return true;
}

bool Renderer::InitInstance()
{
    VkApplicationInfo appInfo = {};
    {
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vibeout";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
    };

    // Get required Vulkan extensions from SDL
    uint nbExtensions = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&nbExtensions);
    if (!extensions)
    {
        printf("Failed to get Vulkan extension count: %s\n", SDL_GetError());
        return false;
    }

    // Create Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    {
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 0;
        createInfo.enabledExtensionCount = nbExtensions;
        createInfo.ppEnabledExtensionNames = extensions;
    };

    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
    {
        printf("Failed to create Vulkan instance\n");
        return false;
    }

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

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = std::size(deviceExtensions);
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(_physicalDevice, &deviceInfo, nullptr, &_device) != VK_SUCCESS)
    {
        std::cerr << "Failed to create logical device!\n";
        return false;
    }

    vkGetDeviceQueue(_device, _graphicsQueueFamilyIndex, 0, &_graphicsQueue);

    return true;
}
