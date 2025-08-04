// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class Renderer
{
public:
	Renderer(SDL_Window& window, bool& result);
	~Renderer();

private:
	bool Init();
	bool InitInstance();
	bool InitSurface();
	bool InitPhysicalDevice();
	bool InitQueueFamilies();
	bool InitLogicalDevice();

	SDL_Window& _window;
	VkInstance _instance = nullptr;
	VkSurfaceKHR _surface = nullptr;
	VkPhysicalDevice _physicalDevice = nullptr;
	VkDevice _device = nullptr;
	uint32 _graphicsQueueFamilyIndex = uint32(-1);
	VkQueue _graphicsQueue = nullptr;
};
