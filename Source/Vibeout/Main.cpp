// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include <SDL3/SDL_main.h>

using uint = unsigned int;

int main(int argc, char* argv[])
{
    SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Vibeout", 1024, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!window)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize Vulkan
    VkInstance instance = VK_NULL_HANDLE;

    // Get required Vulkan extensions from SDL
    uint nbExtensions = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&nbExtensions);
    if (!extensions)
    {
        printf("Failed to get Vulkan extension count: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create Vulkan instance
    VkInstanceCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabledLayerCount = 0,
        .enabledExtensionCount = nbExtensions,
        .ppEnabledExtensionNames = extensions,
    };

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        printf("Failed to create Vulkan instance\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create Vulkan surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
    {
        printf("Failed to create Vulkan surface: %s\n", SDL_GetError());
        vkDestroyInstance(instance, nullptr);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event e;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
            if (e.type == SDL_EVENT_QUIT)
                quit = 1;
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
